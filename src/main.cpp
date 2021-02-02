#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>

#include "engine.h"

using namespace foundation;

int main(int argc, char *argv[]) {
    SDL_SetMainReady();

    return engine::init_engine(argc, argv);
}
