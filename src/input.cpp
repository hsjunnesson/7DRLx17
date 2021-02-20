#pragma warning(push, 0)
#include <SDL2/SDL.h>
#pragma warning(pop)

#include "input.h"

namespace input {

void process_events(Array<InputCommand> &input_commands) {
    SDL_Event event;

    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            InputCommand input_command;
            input_command.action = Action::Quit;
            array::push_back(input_commands, input_command);
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
                InputCommand input_command;
                input_command.action = Action::Quit;
                array::push_back(input_commands, input_command);
                break;
            }
            default:
                break;
            }
        } else if (event.type == SDL_MOUSEWHEEL) {
            if (event.wheel.y != 0) {
                InputCommand input_command;

                if (event.wheel.y > 0) {
                    input_command.action = Action::ZoomIn;
                } else if (event.wheel.y < 0) {
                    input_command.action = Action::ZoomOut;
                }

                array::push_back(input_commands, input_command);
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            InputCommand input_command;
            input_command.action = Action::Mouse;
            input_command.mouse_state.mouse_position.x = event.motion.x;
            input_command.mouse_state.mouse_position.y = event.motion.y;
            input_command.mouse_state.mouse_relative_motion.x = event.motion.xrel;
            input_command.mouse_state.mouse_relative_motion.y = event.motion.yrel;

            input_command.mouse_state.mouse_left_state = MouseButtonState::None;
            input_command.mouse_state.mouse_right_state = MouseButtonState::None;

            if (event.motion.state & SDL_BUTTON_LMASK) {
                input_command.mouse_state.mouse_left_state = MouseButtonState::Repeated;
            }

            if (event.motion.state & SDL_BUTTON_RMASK) {
                input_command.mouse_state.mouse_right_state = MouseButtonState::Repeated;
            }

            array::push_back(input_commands, input_command);
        } else if (event.type == SDL_MOUSEBUTTONDOWN  || event.type == SDL_MOUSEBUTTONUP) {
            InputCommand input_command;
            input_command.action = Action::Mouse;
            input_command.mouse_state.mouse_position.x = event.button.x;
            input_command.mouse_state.mouse_position.y = event.button.y;
            input_command.mouse_state.mouse_relative_motion.x = 0;
            input_command.mouse_state.mouse_relative_motion.y = 0;

            input_command.mouse_state.mouse_left_state = MouseButtonState::None;
            input_command.mouse_state.mouse_right_state = MouseButtonState::None;

            if (event.button.button == SDL_BUTTON_LEFT) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    input_command.mouse_state.mouse_left_state = MouseButtonState::Pressed;
                } else {
                    input_command.mouse_state.mouse_left_state = MouseButtonState::Released;
                }
            }

            if (event.button.button == SDL_BUTTON_RIGHT) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    input_command.mouse_state.mouse_right_state = MouseButtonState::Pressed;
                } else {
                    input_command.mouse_state.mouse_right_state = MouseButtonState::Released;
                }
            }

            array::push_back(input_commands, input_command);
        }
    }
}

} // namespace input
