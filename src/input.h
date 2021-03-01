#pragma once

#pragma warning(push, 0)
#include "array.h"
#include "hash.h"
#include "memory.h"
#pragma warning(pop)

namespace input {
using namespace foundation;

enum class Action {
    Quit,
    ZoomIn,
    ZoomOut,
    MouseMoved,
    MouseTrigger,
    MoveDown,
    MoveLeft,
    MoveRight,
    MoveUp,
};

enum class TriggerState {
    None,
    Pressed,
    Released,
    Repeated,
};

struct Vector2 {
    int32_t x;
    int32_t y;
};

struct KeyEvent {
    TriggerState trigger_state;
};

struct MouseEvent {
    Vector2 mouse_position;
    Vector2 mouse_relative_motion;
    TriggerState mouse_left_state;
    TriggerState mouse_right_state;
};

struct InputCommand {
    Action action;

    union {
        KeyEvent key_event;
        MouseEvent mouse_event;
    };
};

// The input system that keeps track of the state of inputs.
struct Input {
    Input(Allocator &allocator);

    Allocator &allocator;

    // Current set of input commands.
    Array<InputCommand> input_commands;

    // Global hash to keep track of which keys are pressed.
    Hash<bool> keys_pressed;
};

// Processes all pending input events.
void process_events(Input &input);

} // namespace input
