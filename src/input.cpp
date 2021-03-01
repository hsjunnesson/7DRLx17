#pragma warning(push, 0)
#include <SDL2/SDL.h>
#pragma warning(pop)

#include "input.h"
#include "log.h"

namespace input {

using namespace foundation;

Input::Input(Allocator &allocator)
: allocator(allocator)
, input_commands(Array<InputCommand>(allocator))
, keys_pressed(Hash<bool>(allocator)) {}

void process_events(Input &input) {
    SDL_Event event;

    array::clear(input.input_commands);

    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            InputCommand input_command;
            input_command.action = Action::Quit;
            array::push_back(input.input_commands, input_command);
        } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            InputCommand input_command;

            if (event.type == SDL_KEYDOWN) {
                if (hash::get(input.keys_pressed, event.key.keysym.sym, false)) {
                    input_command.key_event.trigger_state = input::TriggerState::Repeated;
                } else {
                    input_command.key_event.trigger_state = input::TriggerState::Pressed;
                    hash::set(input.keys_pressed, event.key.keysym.sym, true);
                }
            } else if (event.type == SDL_KEYUP) {
                input_command.key_event.trigger_state = input::TriggerState::Released;
                hash::remove(input.keys_pressed, event.key.keysym.sym);
            }

            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
                input_command.action = Action::Quit;
                break;
            }
            case SDLK_KP_2:
            case SDLK_DOWN: {
                input_command.action = Action::MoveDown;
                break;
            }
            case SDLK_KP_4:
            case SDLK_LEFT: {
                input_command.action = Action::MoveLeft;
                break;
            }
            case SDLK_KP_6:
            case SDLK_RIGHT: {
                input_command.action = Action::MoveRight;
                break;
            }
            case SDLK_KP_8:
            case SDLK_UP: {
                input_command.action = Action::MoveUp;
                break;
            }
            default:
                continue;
                break;
            }

            array::push_back(input.input_commands, input_command);
        } else if (event.type == SDL_MOUSEWHEEL) {
            if (event.wheel.y != 0) {
                InputCommand input_command;

                if (event.wheel.y > 0) {
                    input_command.action = Action::ZoomIn;
                } else if (event.wheel.y < 0) {
                    input_command.action = Action::ZoomOut;
                }

                array::push_back(input.input_commands, input_command);
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            InputCommand input_command;
            input_command.action = Action::MouseMoved;
            input_command.mouse_event.mouse_position.x = event.motion.x;
            input_command.mouse_event.mouse_position.y = event.motion.y;
            input_command.mouse_event.mouse_relative_motion.x = event.motion.xrel;
            input_command.mouse_event.mouse_relative_motion.y = event.motion.yrel;

            input_command.mouse_event.mouse_left_state = TriggerState::None;
            input_command.mouse_event.mouse_right_state = TriggerState::None;

            if (event.motion.state & SDL_BUTTON_LMASK) {
                input_command.mouse_event.mouse_left_state = TriggerState::Repeated;
            }

            if (event.motion.state & SDL_BUTTON_RMASK) {
                input_command.mouse_event.mouse_right_state = TriggerState::Repeated;
            }

            array::push_back(input.input_commands, input_command);
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            InputCommand input_command;
            input_command.action = Action::MouseTrigger;
            input_command.mouse_event.mouse_position.x = event.button.x;
            input_command.mouse_event.mouse_position.y = event.button.y;
            input_command.mouse_event.mouse_relative_motion.x = 0;
            input_command.mouse_event.mouse_relative_motion.y = 0;

            input_command.mouse_event.mouse_left_state = TriggerState::None;
            input_command.mouse_event.mouse_right_state = TriggerState::None;

            if (event.button.button == SDL_BUTTON_LEFT) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    input_command.mouse_event.mouse_left_state = TriggerState::Pressed;
                } else {
                    input_command.mouse_event.mouse_left_state = TriggerState::Released;
                }
            }

            if (event.button.button == SDL_BUTTON_RIGHT) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    input_command.mouse_event.mouse_right_state = TriggerState::Pressed;
                } else {
                    input_command.mouse_event.mouse_right_state = TriggerState::Released;
                }
            }

            array::push_back(input.input_commands, input_command);
        }
    }
}

} // namespace input
