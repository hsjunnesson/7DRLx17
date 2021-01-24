#include "canvas.hpp"
#include "texture.hpp"


Canvas::Canvas(RendererShPtr renderer)
  : _sdl_renderer(renderer) {
  SDL_Texture *texture = loadTexture(renderer.get(), "assets/colored_transparent.png");
  _tex.reset(texture);
}

void Canvas::render() {
  SDL_Renderer *renderer = _sdl_renderer.get();
  SDL_Rect tile;
  tile.x = 36;
  tile.y = 36;
  tile.w = 16;
  tile.h = 16;

  renderTextureTile(renderer, _tex.get(), &tile, 32, 32, 8);
}
