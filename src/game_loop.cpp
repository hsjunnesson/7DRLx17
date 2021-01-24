#include "game_loop.hpp"

static const double UPDATE_RATE = 60;
static const double FIXED_DELTATIME = 1.0 / UPDATE_RATE;
static const Uint32 DESIRED_FRAMETIME = 1000 / (Uint32)UPDATE_RATE;
static const int UPDATE_MULTIPLICITY = 1;

GameLoop::GameLoop(Window &window, World &world, InputManager &input_manager)
  : _window(window)
  , _world(world)
  , _input_manager(input_manager)
  {}

void GameLoop::run() {
  Uint32 prev_frame_time = SDL_GetTicks();
  Uint32 frame_accumulator = 0;

  bool running = true;
  while (running) {
    Uint32 current_frame_time = SDL_GetTicks();
    Uint32 delta_time = current_frame_time - prev_frame_time;
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
    if (_input_manager.process_events()) {
      running = false;
      continue;
    }

    while (frame_accumulator >= DESIRED_FRAMETIME * UPDATE_MULTIPLICITY) {
      for (int i = 0; i < UPDATE_MULTIPLICITY; i++) {
        // Update
        _world.update(current_frame_time, FIXED_DELTATIME);
        frame_accumulator -= DESIRED_FRAMETIME;
      }
    }

    // Render
    _window.clear();
    _window.render();
  }
}
