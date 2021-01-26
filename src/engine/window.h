#pragma once

#include <SDL.h>

namespace engine {
    struct Window {
        explicit Window(SDL_Window* window, SDL_Renderer* renderer)
        : m_window(window)
        , m_renderer(renderer)
        {}
        
        SDL_Window*     m_window;
        SDL_Renderer*   m_renderer;
    };

    /// Clears the window.
    void clear_window(Window& window);

    /// Renders the window
    void render_window(Window& window);
}
