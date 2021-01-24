#pragma once

#include <SDL.h>

#include "world.hpp"


class Renderer {
public:

  Renderer(SDL_Window &window, SDL_Renderer &renderer);

public:

  void render(const World &world);

  SDL_Window &_window;
  SDL_Renderer &_renderer;

};
