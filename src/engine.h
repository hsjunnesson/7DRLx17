#pragma once

#include <stdint.h>
#include <SDL.h>

#include "world.h"

namespace engine {
    struct Window {
        explicit Window(SDL_Window *window, SDL_Renderer *renderer)
        : window(window)
        , renderer(renderer)
        {}
        
        SDL_Window      *window;
        SDL_Renderer    *renderer;
    };

    struct Tile {
        int16_t     x;
        int16_t     y;
        uint16_t    tile;
    };

    struct Engine {
        explicit Engine(Window &window, world::World &world)
        : frames(0)
        , window(window)
        , world(world)
        {}

        uint64_t        frames;
        Window          &window;
        world::World    &world;
    };

    /// Clears the window.
    void clear_window(Window &window);

    /// Renders the window
    void render_window(Window &window);

    // Runs the engine, returns the exit code.
    int init_engine(int argc, char *argv[]);
}
