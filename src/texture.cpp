#include <SDL.h>
#include <SDL_image.h>

#include "texture.h"
#include "log.h"

namespace texture {
	Atlas::Atlas(SDL_Renderer *renderer, const char *filename, int tile_size, int gutter)
	: tile_size(tile_size)
	, gutter(gutter)
	{
		sdl_texture = load(renderer, filename);
		SDL_QueryTexture(sdl_texture, nullptr, nullptr, &w, &h);
	}

    Atlas::~Atlas() {
		if (sdl_texture) {
			SDL_DestroyTexture(sdl_texture);
		}
	}

	SDL_Texture *load(SDL_Renderer *renderer, const char *filename) {
		SDL_Texture *texture = nullptr;
		texture = IMG_LoadTexture(renderer, filename);
		if (!texture) {
			log_fatal("Could not load texture: %s", SDL_GetError());
		}
		return texture;
	}

	void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y, int scale) {
		int w, h;
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		rect.w = w * scale;
		rect.h = h  * scale;

		render_texture_rect(renderer, texture, &rect);
	}

	void render_texture_rect(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect *rect) {
		if (SDL_RenderCopy(renderer, texture, nullptr, rect)) {
			log_error("Error in renderTextureRect: %s", SDL_GetError());
		}
	}

	void render_texture_tile(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect *tile, int x, int y, int scale) {
		int w, h;
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

		SDL_Rect destination;
		destination.x = x;
		destination.y = y;
		destination.w = tile->w * scale;
		destination.h = tile->h * scale;

		if (SDL_RenderCopy(renderer, texture, tile, &destination)) {
			log_error("Error in renderTextureTile: %s", SDL_GetError());
		}
	}
}
