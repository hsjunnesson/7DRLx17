#include <SDL.h>

#include "renderer.hpp"
#include "world.hpp"

#include <stdio.h>


Renderer::Renderer(SDL_Window &window, SDL_Renderer &renderer)
  : _window(window)
  , _renderer(renderer)
  {}

void Renderer::render(const World &world) {
  printf("Got here");
}
