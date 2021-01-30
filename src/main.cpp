#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <SDL.h>
#include "memory.h"
#include "string_stream.h"
#pragma warning( pop )

#include "engine.h"

using namespace foundation;

int main(int argc, char *argv[]) {
    return engine::init_engine(argc, argv);
}
