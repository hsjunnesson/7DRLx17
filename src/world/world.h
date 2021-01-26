#pragma once

#include <SDL.h>

namespace roguelike {
    struct World {
    };

    /**
     * @brief Updates the world
     *
     * @param world The world to update
     * @param t The current time
     * @param dt The delta time since last update
     */
    void update_world(World world, uint32_t t, double dt);
}
