#define SDL_MAIN_HANDLED

#include <fstream>
#include <string>

#include "engine.h"
#include "config.h"

#pragma warning(push, 0)
#include <SDL2/SDL.h>
#include "proto/engine.pb.h"
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
