#pragma once

#include <stdint.h>

#pragma warning(push, 0)
#include "memory.h"
#include <SDL2/SDL.h>
#pragma warning(pop)

#include "game.h"
#include "gui.h"

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
    Engine(Allocator &allocator, Window &window, game::Game &game, gui::Gui &gui)
    : allocator(allocator)
    , frames(0)
    , window(window)
    , game(game)
    , gui(gui)
    , clear_color(SDL_Color{0, 0, 0, 255}) {}

    Allocator &allocator;
    uint64_t frames;
    Window &window;
    game::Game &game;
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
