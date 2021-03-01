#include <fstream>

#pragma warning(push, 0)
#include "engine.h"
#include "memory.h"
#include "proto/engine.pb.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#pragma warning(pop)

#include "input.h"
#include "log.h"

namespace engine {
using namespace foundation;

static const double Update_Rate = 60;
static const uint32_t Desired_Frametime = 1000 / (uint32_t)Update_Rate;

void clear(Window &window, SDL_Color &clear_color) {
    if (SDL_SetRenderDrawColor(window.renderer, clear_color.r, clear_color.g, clear_color.b, clear_color.a)) {
        log_error("SDL_SetRenderDrawColor: %s", SDL_GetError());
    }

    if (SDL_RenderClear(window.renderer)) {
        log_error("SDL_RenderClear: %s", SDL_GetError());
    }

    if (SDL_SetRenderDrawColor(window.renderer, 1, 1, 1, 255)) {
        log_error("SDL_SetRenderDrawColor: %s", SDL_GetError());
    }
}

void render(Window &window) {
    SDL_RenderPresent(window.renderer);
}

int run(Engine &engine) {
    uint32_t prev_frame_time = SDL_GetTicks();

    bool running = true;
    int exit_code = 0;

    uint32_t current_frame_time = SDL_GetTicks();
    uint32_t delta_time = current_frame_time - prev_frame_time;

    while (running) {
        // Process events
        input::process_events(engine.input);
        for (uint32_t i = 0; i < array::size(engine.input.input_commands); ++i) {
            game::on_input(engine.game, engine.input.input_commands[i]);
        }

        // Update
        game::update(engine.game, current_frame_time, delta_time);
        gui::update(engine.gui, current_frame_time, delta_time);

        // Check terminate
        if (engine.game.game_state == game::GameState::Terminate) {
            running = false;
            break;
        }

        // Render
        engine::clear(engine.window, engine.clear_color);
        game::render(engine.game, engine.window.renderer);
        gui::render(engine.gui, engine.window.renderer);
        engine::render(engine.window);

        ++engine.frames;

        // Calculate frame times
        current_frame_time = SDL_GetTicks();
        delta_time = current_frame_time - prev_frame_time;
        prev_frame_time = current_frame_time;

        // Handle anomalies
        if (delta_time < 0) {
            delta_time = 0;
        }

        if (delta_time < Desired_Frametime) {
            uint32_t delay = Desired_Frametime - delta_time;
            SDL_Delay(delay);
            delta_time += delay;
        }
    }

    return exit_code;
}

int init_engine(EngineParams &params) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        log_fatal("SDL_Init: %s", SDL_GetError());
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        log_fatal("IMG_Init: %s", IMG_GetError());
    }

    foundation::memory_globals::init();
    Allocator &allocator = memory_globals::default_allocator();

    SDL_Rect window_rect;
    window_rect.x = 100;
    window_rect.y = 100;
    window_rect.w = params.width();
    window_rect.h = params.height();

    SDL_Window *sdl_window = SDL_CreateWindow("Roguelike", window_rect.x, window_rect.y, window_rect.w, window_rect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!sdl_window) {
        log_fatal("Couldn't create window: %s", SDL_GetError());
    }

    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl_renderer) {
        log_fatal("Couldn't create renderer: %s", SDL_GetError());
    }

    int render_scale = params.render_scale();
    if (SDL_RenderSetScale(sdl_renderer, (float)render_scale, (float)render_scale)) {
        log_error("Error in SDL_RenderSetScale: %s", SDL_GetError());
    }

    Window window = Window(sdl_window, sdl_renderer);

    game::Game *game = MAKE_NEW(allocator, game::Game, allocator, sdl_renderer, params.atlas().c_str());
    if (!game) {
        log_fatal("Couldn't create game");
    }

    gui::Gui *gui = MAKE_NEW(allocator, gui::Gui, allocator);
    if (!gui) {
        log_fatal("Couldn't greate gui");
    }

    input::Input *input = MAKE_NEW(allocator, input::Input, allocator);

    Engine *engine = MAKE_NEW(allocator, Engine, allocator, window, *game, *gui, *input);
    if (!engine) {
        log_fatal("Couldn't create engine");
    }

    SDL_Color clear_color;
    clear_color.r = (Uint8)params.clear_color().r();
    clear_color.g = (Uint8)params.clear_color().g();
    clear_color.b = (Uint8)params.clear_color().b();
    clear_color.a = (Uint8)params.clear_color().a();

    engine->clear_color = clear_color;

    int exit_code = run(*engine);

    MAKE_DELETE(allocator, Engine, engine);
    MAKE_DELETE(allocator, Game, game);
    MAKE_DELETE(allocator, Gui, gui);
    MAKE_DELETE(allocator, Input, input);

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    foundation::memory_globals::shutdown();
    SDL_Quit();

    return exit_code;
}

} // namespace engine
