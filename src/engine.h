#pragma once

#include <stdint.h>
#include <SDL2/SDL.h>

#include "world.h"
#include "gui.h"

namespace engine {
    class EngineParams;
    
    struct Window {
        Window(SDL_Window *window, SDL_Renderer *renderer)
        : window(window)
        , renderer(renderer)
        {}
        
        SDL_Window *window;
        SDL_Renderer *renderer;
    };

    struct Engine {
        Engine(Window &window, world::World &world, gui::Gui &gui)
        : frames(0)
        , window(window)
        , world(world)
        , gui(gui)
        , clear_color(SDL_Color {0, 0, 0, 255})
        {}

        uint64_t frames;
        Window &window;
        world::World &world;
        gui::Gui &gui;
        SDL_Color clear_color;
    };

    /// Clears the window.
    void clear(Window &window, SDL_Color &clear_color);

    /// Renders the window
    void render(Window &window);

    // Runs the engine, returns the exit code.
    int init_engine(EngineParams &params);
}
