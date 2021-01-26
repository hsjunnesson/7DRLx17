#pragma once

#include "input_manager.h"
#include "world.h"
#include "window.h"
#include "input_manager.h"

namespace engine {
    struct GameLoop {
        explicit GameLoop(Window &window, World &world, InputManager &inputManager)
        : m_window(window)
        , m_world(world)
        , m_input_manager(input_manager)
        {}

        Window&         m_window;
        World&          m_world;
        InputManager&   m_input_manager;
    };

}
