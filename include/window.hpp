#pragma once

#include <SDL.h>


#include "canvas.hpp"
#include "SDL_Utils.hpp"


class Window {
public:

	Window(SDL_Rect rect, SDL_Window &window, SDL_Renderer &renderer);
	
	~Window();

	/// Clears the window.
	void clear();

	/// Renders the window
	void render();

private:

	SDL_Rect _rect;
	WindowPtr _sdl_window;
	RendererShPtr _sdl_renderer;
	std::unique_ptr<Canvas> _canvas;

};
