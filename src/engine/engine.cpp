#include <SDL.h>
#include <SDL_image.h>

#include "engine.h"
#include "memory.h"
#include "log.h"
#include "input_manager.h"

static const double UPDATE_RATE = 60;
static const double FIXED_DELTATIME = 1.0 / UPDATE_RATE;
static const uint32_t DESIRED_FRAMETIME = 1000 / (uint32_t)UPDATE_RATE;
static const int UPDATE_MULTIPLICITY = 1;

namespace engine {
   using namespace foundation;

    int run(Engine& engine) {
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
                    update_world(engine.m_world, current_frame_time, FIXED_DELTATIME);
                    frame_accumulator -= DESIRED_FRAMETIME;
                }
            }

            // Render
            clear_window(engine.m_window);
            render_window(engine.m_window);

            ++engine.m_frames;
        }

        return exit_code;
    }

    int run_loop(int argc, char *argv[]) {
        Allocator &a = memory_globals::default_allocator();

        SDL_Rect rect;
	    rect.x = 100;
	    rect.y = 100;
	    rect.w = 640;
	    rect.h = 480;

    	SDL_Window *sdl_window = SDL_CreateWindow("Roguelike", rect.x, rect.y, rect.w, rect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!sdl_window) {
            log_fatal("Couldn't create window: %s", SDL_GetError());
            return 1;
        }

    	SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!sdl_renderer) {
            log_fatal("Couldn't create renderer: %s", SDL_GetError());
            return 1;
        }

        Window window = Window(sdl_window, sdl_renderer);

        roguelike::World *world = MAKE_NEW(a, roguelike::World);
        if (!world) {
            log_fatal("Couldn't create world");
            return 1;
        }

        Engine *engine = MAKE_NEW(a, Engine, window, *world);
        if (!engine) {
            log_fatal("Couldn't create engine");
            return 1;
        }

        int exit_code = run(*engine);

        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(sdl_window);
        MAKE_DELETE(a, Engine, engine);
        MAKE_DELETE(a, World, world);

        return exit_code;
    }
}

int engine_main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		log_fatal("SDL_Init: %s", SDL_GetError());
        return 1;
	}

	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		log_fatal("IMG_Init: %s", IMG_GetError());
        return 1;
	}

    foundation::memory_globals::init();

    int exit_code = engine::run_loop(argc, argv);

    foundation::memory_globals::shutdown();
	SDL_Quit();

    return exit_code;
}
