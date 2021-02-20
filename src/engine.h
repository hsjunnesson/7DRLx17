#pragma once

#include <stdint.h>

#pragma warning(push, 0)
#include "memory.h"
#include <SDL2/SDL.h>
#pragma warning(pop)

#include "gui.h"
#include "world.h"

namespace engine {
using namespace foundation;

class EngineParams;

struct Window {
    Window(SDL_Window *window, SDL_Renderer *renderer)
    : window(window)
    , renderer(renderer) {}

    SDL_Window *window;
    SDL_Renderer *renderer;
};

struct Engine {
    Engine(Allocator &allocator, Window &window, world::World &world, gui::Gui &gui)
    : allocator(allocator)
    , frames(0)
    , window(window)
    , world(world)
    , gui(gui)
    , clear_color(SDL_Color{0, 0, 0, 255}) {}

    Allocator &allocator;
    uint64_t frames;
    Window &window;
    world::World &world;
    gui::Gui &gui;
    SDL_Color clear_color;
};

/// Clears the window.
void clear(Window &window, SDL_Color &clear_color);

/// Renders the window
void render(Window &window);

// Runs the engine, returns the exit code.
int init_engine(EngineParams &params);

} // namespace engine
