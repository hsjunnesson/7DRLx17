#define SDL_MAIN_HANDLED

#include <fstream>
#include <string>

#include "config.h"
#include "engine.h"

#pragma warning(push, 0)
#include "proto/engine.pb.h"
#include <SDL2/SDL.h>
#pragma warning(pop)

using namespace foundation;
using namespace google::protobuf;

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SDL_SetMainReady();

    engine::EngineParams params;
    config::read("assets/engine_params.json", &params);

    return engine::init_engine(params);
}
