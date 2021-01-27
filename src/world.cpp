#include "world.h"
#include "texture.h"
#include "log.h"

namespace world {
    World::World(SDL_Renderer *renderer)
    : m_renderer(renderer)
    , m_atlas(texture::load_texture(m_renderer, "assets/colored_transparent.png"))
    {}

    World::~World() {
        if (m_atlas) {
            SDL_DestroyTexture(m_atlas);
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

        texture::render_texture_tile(world.m_renderer, world.m_atlas, &tile, 32, 32, 2);
    }

}

