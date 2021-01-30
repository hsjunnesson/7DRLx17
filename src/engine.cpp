#include <SDL.h>
#include <SDL_image.h>

#include "engine.h"
#include "memory.h"
#include "log.h"
#include "input.h"

static const double UPDATE_RATE = 60;
static const double FIXED_DELTATIME = 1.0 / UPDATE_RATE;
static const uint32_t DESIRED_FRAMETIME = 1000 / (uint32_t)UPDATE_RATE;
static const int UPDATE_MULTIPLICITY = 1;

namespace engine {
    using namespace foundation;

    void clear_window(Window &window, SDL_Color &clear_color) {
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

    void render_window(Window& window) {
	    SDL_RenderPresent(window.renderer);
    }

    int run(Engine &engine) {
        uint32_t prev_frame_time = SDL_GetTicks();
        uint32_t frame_accumulator = 0;

        bool running = true;
        int exit_code = 0;

        while (running) {
            uint32_t current_frame_time = SDL_GetTicks();
            uint32_t delta_time = current_frame_time - prev_frame_time;
            prev_frame_time = current_frame_time;

            // Handle anomalies
            
            if (delta_time > DESIRED_FRAMETIME * 8) { // Ignore extra slow frames
                delta_time = DESIRED_FRAMETIME;
            }
            
            if (delta_time < 0) {
                delta_time = 0;
            }

            frame_accumulator += delta_time;

            // Spiral of death protection
            if (frame_accumulator > DESIRED_FRAMETIME * 8) {
                frame_accumulator = 0;
                delta_time = DESIRED_FRAMETIME;
            }

            // Process events, signals quit?
            // False for keep running
            if (input::process_events()) {
                running = false;
                continue;
            }

            while (frame_accumulator >= DESIRED_FRAMETIME * UPDATE_MULTIPLICITY) {
                for (int i = 0; i < UPDATE_MULTIPLICITY; i++) {
                    // Update
                    world::update_world(engine.world, current_frame_time, FIXED_DELTATIME);
                    frame_accumulator -= DESIRED_FRAMETIME;
                }
            }

            // Render
            clear_window(engine.window, engine.clear_color);
            world::render_world(engine.world);
            render_window(engine.window);

            ++engine.frames;
        }

        return exit_code;
    }

    int init_engine(int argc, char *argv[]) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            log_fatal("SDL_Init: %s", SDL_GetError());
        }

        if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
            log_fatal("IMG_Init: %s", IMG_GetError());
        }

        foundation::memory_globals::init();
        Allocator &allocator = memory_globals::default_allocator();

        SDL_Rect rect;
        rect.x = 100;
        rect.y = 100;
        rect.w = 640;
        rect.h = 480;

        SDL_Window *sdl_window = SDL_CreateWindow("Roguelike", rect.x, rect.y, rect.w, rect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!sdl_window) {
            log_fatal("Couldn't create window: %s", SDL_GetError());
        }

        SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!sdl_renderer) {
            log_fatal("Couldn't create renderer: %s", SDL_GetError());
        }

        SDL_RenderSetScale(sdl_renderer, 4.0, 4.0);

        Window window = Window(sdl_window, sdl_renderer);

        world::World *world = MAKE_NEW(allocator, world::World, allocator, sdl_renderer);
        if (!world) {
            log_fatal("Couldn't create world");
        }

        Engine *engine = MAKE_NEW(allocator, Engine, window, *world);
        if (!engine) {
            log_fatal("Couldn't create engine");
        }

        int exit_code = run(*engine);

        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(sdl_window);
        MAKE_DELETE(allocator, Engine, engine);
        MAKE_DELETE(allocator, World, world);
        foundation::memory_globals::shutdown();
        SDL_Quit();

        return exit_code;
    }
}
