#pragma once

#include <SDL.h>

#include "hash.h"
#include "memory.h"
#include "texture.h"

namespace world {
    using namespace foundation;

    // Max width of the world.
    static const uint64_t Max_Width = 256;

    // Max height of the world.
    static const uint64_t Max_Height = 256;

    // The game world.
    struct World {
        World(Allocator &allocator, SDL_Renderer *renderer);
        ~World();

        Allocator       &allocator;
        SDL_Renderer    *renderer;
        Hash<int>       tiles;
        texture::Atlas  *atlas;
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

    /**
     * @brief Returns the index of an x, y coordinate
     * 
     * @param x The x coord
     * @param y The y coord
     * @return constexpr uint64_t The index.
     */
    constexpr uint64_t index(uint64_t const x, uint64_t const y) {
        return x + Max_Width * y;
    }

    /**
     * @brief Calculates the x, y coordinates based on an index.
     * 
     * @param index The index.
     * @param x The pass-by-reference x coord to calculate.
     * @param y The pass-by-reference y coord to calculate.
     */
    constexpr void coord(uint64_t const index, uint64_t &x, uint64_t &y) {
        x = index % Max_Width;
        y = index / Max_Width;
    }
}
