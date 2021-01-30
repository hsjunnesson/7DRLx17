#include "world.h"
#include "texture.h"
#include "log.h"


namespace world {
    static const uint64_t Max_Tiles = Max_Width * Max_Height;
    
    World::World(Allocator &allocator, SDL_Renderer *renderer)
    : allocator(allocator)
    , renderer(renderer)
    , tiles(Hash<int>(allocator))
    , atlas(texture::create_atlas(allocator, renderer, "assets/colored_tilemap_atlas.json"))
    {
        hash::reserve(tiles, Max_Tiles);
        hash::set(tiles, index(0, 0), 25);
        hash::set(tiles, index(1, 0), 26);
        hash::set(tiles, index(2, 0), 26);
        hash::set(tiles, index(0, 1), 27);
        hash::set(tiles, index(1, 1), 28);
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

        int w, h;
        SDL_GetRendererOutputSize(world.renderer, &w, &h);

        int tile_size = world.atlas->tile_size;

        SDL_Rect source;
        source.x = 0;
        source.y = 0;
        source.w = tile_size;
        source.h = tile_size;

		SDL_Rect destination;
		destination.x = 0;
		destination.y = 0;
		destination.w = tile_size;
		destination.h = tile_size;

		if (SDL_RenderCopy(world.renderer, world.atlas->texture, &source, &destination)) {
			log_error("Error in SDL_RenderCopy: %s", SDL_GetError());
		}
    }
}
