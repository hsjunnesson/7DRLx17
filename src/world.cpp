#include "world.h"
#include "texture.h"
#include "log.h"

namespace world {
    World::World(SDL_Renderer *renderer)
    : renderer(renderer)
    , atlas(texture::load_texture(renderer, "assets/colored_transparent.png"))
    {}

    World::~World() {
        if (atlas) {
            SDL_DestroyTexture(atlas);
        }
    }

    void update_world(World &world, uint32_t t, double dt) {
    }

    void render_world(World &world) {
        SDL_Rect tile;
        tile.x = 36;
        tile.y = 36;
        tile.w = 16;
        tile.h = 16;

        texture::render_texture_tile(world.renderer, world.atlas, &tile, 32, 32, 2);
    }

}

