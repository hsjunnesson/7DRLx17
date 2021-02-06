#pragma once

#include <SDL2/SDL.h>

#include "memory.h"

namespace gui {
    using namespace foundation;

    struct Gui {
        Gui(Allocator &allocator, SDL_Renderer *renderer);

        Allocator &allocator;

    };

    /**
     * @brief Renders the world
     *
     * @param world The world to render
     * @param renderer The SDL renderer.
     */
    void render(Gui &gui, SDL_Renderer *renderer);


}
