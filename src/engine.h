#pragma once

#include <stdint.h>
#include <SDL.h>

#include "world.h"

namespace engine {
    struct Window {
        Window(SDL_Window *window, SDL_Renderer *renderer)
        : window(window)
        , renderer(renderer)
        {}
        
        SDL_Window *window;
        SDL_Renderer *renderer;
    };

    struct Engine {
        Engine(Window &window, world::World &world)
        : frames(0)
        , window(window)
        , world(world)
        , clear_color(SDL_Color {0, 0, 0, 255})
        {}

        uint64_t frames;
        Window &window;
        world::World &world;
        SDL_Color clear_color;
    };

    /// Clears the window.
    void clear_window(Window &window, SDL_Color &clear_color);

    /// Renders the window
    void render_window(Window &window);

    // Runs the engine, returns the exit code.
    int init_engine(int argc, char *argv[]);
}
