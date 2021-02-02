#include <cstdlib>

#include "world.h"
#include "texture.h"
#include "log.h"


namespace world {
    static const uint64_t Max_Tiles = Max_Width * Max_Height;
    
    World::World(Allocator &allocator, SDL_Renderer *renderer, const char *atlas_config_filename)
    : allocator(allocator)
    , renderer(renderer)
    , tiles(Hash<Tile>(allocator))
    , atlas(texture::create_atlas(allocator, renderer, atlas_config_filename))
    , x_offset(0)
    , y_offset(0)
    , dirty(false)
    , render_operations(Array<RenderOperation>(allocator))
    {
        hash::reserve(tiles, Max_Tiles);

        for (int y = 0; y < Max_Height; ++y) {
            for (int x = 0; x < Max_Width; ++x) {
                hash::set(tiles, index(x, y, Max_Width), {rand() % 140});
            }
        }

        // hash::set(tiles, index(0, 0, Max_Width), {39});
        // hash::set(tiles, index(1, 0, Max_Width), {40});
        // hash::set(tiles, index(2, 0, Max_Width), {40});
        // hash::set(tiles, index(3, 0, Max_Width), {39, SDL_FLIP_HORIZONTAL});
        // hash::set(tiles, index(0, 1, Max_Width), {40, SDL_FLIP_NONE, 90.0});
        // hash::set(tiles, index(3, 1, Max_Width), {40, SDL_FLIP_NONE, 90.0});
        // hash::set(tiles, index(0, 2, Max_Width), {39, SDL_FLIP_VERTICAL});
        // hash::set(tiles, index(1, 2, Max_Width), {40});
        // hash::set(tiles, index(2, 2, Max_Width), {40});
        // hash::set(tiles, index(3, 2, Max_Width), {39, SDL_FLIP_VERTICAL, -90});

        // hash::set(tiles, index(0, 0, Max_Width), {4});
        // hash::set(tiles, index(1, 0, Max_Width), {18});
        // hash::set(tiles, index(2, 0, Max_Width), {20});
        // hash::set(tiles, index(3, 0, Max_Width), {24});

        // hash::set(tiles, index(0, 1, Max_Width), {62});
        // hash::set(tiles, index(1, 1, Max_Width), {63});
        // hash::set(tiles, index(2, 1, Max_Width), {64});
        // hash::set(tiles, index(3, 1, Max_Width), {65});

        dirty = true;
    }

    World::~World() {
        texture::destroy_atlas(allocator, atlas);
    }

    void update_world(World &world, uint32_t t, double dt) {
    }

    void render_world(World &world) {
        if (!world.atlas) {
            log_fatal("Missing world atlas");
        }

        if (world.dirty) {
            array::clear(world.render_operations);

            int w, h;
            SDL_GetRendererOutputSize(world.renderer, &w, &h);

            int tile_size = world.atlas->tile_size;
            int gutter = world.atlas->gutter;

            for (const Hash<Tile>::Entry *it = hash::begin(world.tiles); it != hash::end(world.tiles); ++it) {
                uint64_t pos_index = it->key;
                Tile tile = it->value;
                int tile_index = tile.index;

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

                array::push_back(world.render_operations, {source, destination, tile.flip, tile.angle});
            }

            world.dirty = false;
        }

        for (const RenderOperation *it = array::begin(world.render_operations); it != array::end(world.render_operations); ++it) {
            SDL_RenderCopyEx(world.renderer, world.atlas->texture, &it->source, &it->destination, it->angle, nullptr, it->flip);
        }
    }
}
