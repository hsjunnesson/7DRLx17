#include "window.h"
#include "log.h"

namespace engine {
    void clear_window(Window& window) {
        if (SDL_SetRenderDrawColor(window.m_renderer, 0, 0, 0, 255)) {
		    log_error("SDL_SetRenderDrawColor: %s", SDL_GetError());
        }

        if (SDL_RenderClear(window.m_renderer)) {
            log_error("SDL_RenderClear: %s", SDL_GetError());
        }
    }

    void render_window(Window& window) {
	    SDL_RenderPresent(window.m_renderer);
    }
}
