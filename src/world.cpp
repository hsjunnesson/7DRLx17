#include <cstdlib>

#include "murmur_hash.h"

#include "world.h"
#include "texture.h"
#include "log.h"


namespace world {
    uint64_t hash(const char *s) {
        return murmur_hash_64(s, strlen(s), 0);
    };

    static const uint64_t Max_Tiles = Max_Width * Max_Height;

    static uint64_t Floor_Hash = world::hash("floor");
    static uint64_t Snake_Hash = world::hash("snake");
    static uint64_t Missing_Hash = world::hash("missing");

    World::World(Allocator &allocator, SDL_Renderer *renderer, const char *atlas_config_filename)
    : allocator(allocator)
    , game_state(GameState::Initializing)
    , atlas(MAKE_NEW(allocator, texture::Atlas, allocator, renderer, atlas_config_filename))
    , x_offset(0)
    , y_offset(0)
    , tiles(Hash<Tile>(allocator))
    {
        hash::reserve(tiles, Max_Tiles);

        if (!hash::has(atlas->tiles_by_name, Missing_Hash)) {
            log_fatal("Atlas does not have the 'missing' named tile.");
        }

        int32_t missing_tile = hash::get(atlas->tiles_by_name, Missing_Hash, 0);

        for (int room_i = 0; room_i < 30; ++room_i) {
            int room_x = rand() % 100;
            int room_y = rand() % 100;

            for (int y = 0; y < 10; ++y) {
                for (int x = 0; x < 10; ++x) {
                    hash::set(tiles, index(room_x + x, room_y + y, Max_Width), {missing_tile});
                }
            }
        }
        
        for (int corridor_i = 0; corridor_i < 10; ++corridor_i) {
            int start_x = rand() % 100;
            int start_y = rand() % 100;

            for (int x = 0; x < 30; ++x) {
                if ((start_x + x) < Max_Width) {
                    hash::set(tiles, index(start_x + x, start_y, Max_Width), {rand() % 140});
                }
            }

            start_x = rand() % 100;
            start_y = rand() % 100;

            for (int y = 0; y < 30; ++y) {
                if ((start_y + y) < Max_Height) {
                    hash::set(tiles, index(start_x, start_y + y, Max_Width), {rand() % 140});
                }
            }
        }

       for (int y = 0; y < Max_Height; ++y) {
           for (int x = 0; x < Max_Width; ++x) {
               hash::set(tiles, index(x, y, Max_Width), {rand() % 140});
           }
       }
    }

    World::~World() {
        if (atlas) {
            MAKE_DELETE(allocator, Atlas, atlas);
        }
    }

    void update(World &world, uint32_t t, double dt) {
    }

    void render(World &world, SDL_Renderer *renderer) {
        if (!world.atlas) {
            log_fatal("Missing world atlas");
        }

        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        int tile_size = world.atlas->tile_size;
        int gutter = world.atlas->gutter;

        for (const Hash<Tile>::Entry *it = hash::begin(world.tiles); it != hash::end(world.tiles); ++it) {
            uint64_t pos_index = it->key;
            Tile tile = it->value;
            int tile_index = tile.index;

            if (tile_index == Floor_Hash) {
                continue;
            }
            
            SDL_Rect source;
            uint64_t source_x, source_y;
            coord(tile_index, source_x, source_y, world.atlas->w_tiles - 1);
            source.x = (int)(source_x * tile_size + source_x * gutter);
            source.y = (int)(source_y * tile_size + source_y * gutter);
            source.w = tile_size;
            source.h = tile_size;

            SDL_Rect destination;
            uint64_t destination_x, destination_y;
            coord(pos_index, destination_x, destination_y, Max_Width);
            destination.x = (int)(destination_x * tile_size);
            destination.y = (int)(destination_y * tile_size);
            destination.w = tile_size;
            destination.h = tile_size;

            SDL_RenderCopyEx(renderer, world.atlas->texture, &source, &destination, tile.angle, nullptr, tile.flip);
        }
    }
}
