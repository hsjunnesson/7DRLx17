#pragma once

#include <stdint.h>

#include "world.h"

namespace engine {
    struct Window {
        explicit Window(SDL_Window* window, SDL_Renderer* renderer)
        : m_window(window)
        , m_renderer(renderer)
        {}
        
        SDL_Window*     m_window;
        SDL_Renderer*   m_renderer;
    };

    struct Engine {
        Engine(Window& window, world::World& world)
        : m_frames(0)
        , m_window(window)
        , m_world(world)
        {}

        uint64_t        m_frames;
        Window&         m_window;
        world::World&   m_world;
    };

    /// Clears the window.
    void clear_window(Window& window);

    /// Renders the window
    void render_window(Window& window);

    // Runs the engine, returns the exit code.
    int init_engine(int argc, char *argv[]);
}
