#pragma once

#include "SDL_Utils.hpp"

#include <SDL.h>
#include <memory>

class Canvas {
public:

	explicit Canvas(RendererShPtr renderer);

	~Canvas() {};

	/// Render the canvas.
	void render();

private:

	RendererShPtr _sdl_renderer;
	std::unique_ptr<SDL_Texture, SDL_Deleter> _tex;

};
