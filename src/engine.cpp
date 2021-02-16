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
        // Update
        world::update(engine.world, current_frame_time, delta_time);
        gui::update(engine.gui, current_frame_time, delta_time);

        // Render
        engine::clear(engine.window, engine.clear_color);
        world::render(engine.world, engine.window.renderer);
        gui::render(engine.gui, engine.window.renderer);
        engine::render(engine.window);

        ++engine.frames;

        // Process events, signals quit?
        // False for keep running
        if (input::process_events()) {
            running = false;
            continue;
        }

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

    world::World *world = MAKE_NEW(allocator, world::World, allocator, sdl_renderer, params.atlas().c_str());
    if (!world) {
        log_fatal("Couldn't create world");
    }

    gui::Gui *gui = MAKE_NEW(allocator, gui::Gui, allocator);
    if (!gui) {
        log_fatal("Couldn't greate gui");
    }

    Engine *engine = MAKE_NEW(allocator, Engine, window, *world, *gui);
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
    MAKE_DELETE(allocator, World, world);
    MAKE_DELETE(allocator, Gui, gui);

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    foundation::memory_globals::shutdown();
    SDL_Quit();

    return exit_code;
}

} // namespace engine
