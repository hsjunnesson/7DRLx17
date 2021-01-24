#pragma once

#include <SDL.h>

#include "input_manager.hpp"
#include "renderer.hpp"
#include "world.hpp"
#include "window.hpp"
#include "input_manager.hpp"
#include "world.hpp"

class GameLoop {
public:

  explicit GameLoop(Window &window, World &world, InputManager &inputManager);

  void run();

private:

  Window &_window;
  World &_world;
  InputManager &_input_manager;
};
