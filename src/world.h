#pragma once

#include <SDL.h>

namespace world {
    struct World {
        explicit World(SDL_Renderer *renderer);
        ~World();

        SDL_Renderer    *renderer;
        SDL_Texture     *atlas;
    };

    /**
     * @brief Updates the world
     *
     * @param world The world to update
     * @param t The current time
     * @param dt The delta time since last update
     */
    void update_world(World &world, uint32_t t, double dt);

    /**
     * @brief Renders the world
     *
     * @param world The world to render
     */
    void render_world(World &world);
}
