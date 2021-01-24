#include "window.hpp"
#include "canvas.hpp"
#include <SDL.h>
#include "log.hpp"


Window::Window(SDL_Rect rect, SDL_Window &window, SDL_Renderer &renderer)
	: _rect(rect)
	, _sdl_window(&window)
	, _sdl_renderer(&renderer)
	, _canvas(std::make_unique<Canvas>(_sdl_renderer))
	{}


// 	sdlWindow_.reset(window, SDL_Deleter());


Window::~Window() {
}

void Window::clear() {
	SDL_Renderer *renderer = _sdl_renderer.get();

  if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
		quit_fmt("SDL_SetRenderDrawColor: %s", SDL_GetError());
	}

	if (SDL_RenderClear(renderer)) {
		quit_fmt("SDL_RenderClear: %s", SDL_GetError());
	}
}

void Window::render() {
	_canvas.get()->render();

	SDL_Renderer *renderer = _sdl_renderer.get();
	SDL_RenderPresent(renderer);
}
