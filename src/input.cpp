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
            input_command.action = InputCommand::Action::Quit;
            array::push_back(input_commands, input_command);
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
                InputCommand input_command;
                input_command.action = InputCommand::Action::Quit;
                array::push_back(input_commands, input_command);
                break;
            }
            default:
                break;
            }
        }
    }
}

} // namespace input
