#pragma once

#pragma warning(push, 0)
#include "array.h"
#include "memory.h"
#include "math_types.h"
#pragma warning(pop)

namespace input {
using namespace foundation;

struct InputCommand {
    enum class Action {
        Quit,
        ZoomIn,
        ZoomOut,
        Scroll,
    };

    Action action;

    union {
        float value_float;
        Vector2 value_vector;
    };
};

void process_events(Array<InputCommand> &input_commands);

}
