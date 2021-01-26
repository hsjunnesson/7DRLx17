#pragma once

#include <stdint.h>

#include "window.h"
#include "../world/world.h"

namespace engine {
    struct Engine {
        Engine(Window& window, roguelike::World& world)
        : m_frames(0)
        , m_window(window)
        , m_world(world)
        {}

        uint64_t            m_frames;
        Window&             m_window;
        roguelike::World&   m_world;
    };

    // Initialize the whole run loop, returns the exit code.
    int run_loop(int argc, char *argv[]);
}
