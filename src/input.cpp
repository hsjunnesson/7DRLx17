#pragma warning(push, 0)
#include <SDL2/SDL.h>
#pragma warning(pop)

#include "input.h"

namespace input {
bool process_events() {
    SDL_Event event;
    bool signal_quit = false;

    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            signal_quit = true;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                signal_quit = true;
                break;
            default:
                break;
            }
        }
    }

    return signal_quit;
}
} // namespace input
