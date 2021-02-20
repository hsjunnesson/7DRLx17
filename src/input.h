#pragma once

#pragma warning(push, 0)
#include "array.h"
#include "memory.h"
#pragma warning(pop)

namespace input {
using namespace foundation;

enum class Action {
    Quit,
    ZoomIn,
    ZoomOut,
    Mouse,
};

enum class MouseButtonState {
    None,
    Pressed,
    Released,
    Repeated,
};

struct Vector2 {
    int32_t x;
    int32_t y;
};

struct MouseState {
    uint32_t timestamp;
    Vector2 mouse_position;
    Vector2 mouse_relative_motion;
    MouseButtonState mouse_left_state;
    MouseButtonState mouse_right_state;
};

union InputCommand {
    Action action;
    MouseState mouse_state;
};

void process_events(Array<InputCommand> &input_commands);

} // namespace input
