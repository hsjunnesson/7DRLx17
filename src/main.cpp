#define SDL_MAIN_HANDLED

#include <fstream>
#include <string>

#include <SDL2/SDL.h>

#include "engine.h"

#include "config.h"
#include "proto/engine.pb.h"

using namespace foundation;
using namespace google::protobuf;

int main(int argc, char *argv[]) {
    SDL_SetMainReady();

    engine::EngineParams params;
    config::read("assets/engine_params.json", &params);

    return engine::init_engine(params);
}
