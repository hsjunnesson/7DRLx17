#include <SDL.h>
#include <SDL_image.h>
#include "texture.hpp"
#include "log.hpp"

SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *filename) {
	SDL_Texture *texture = NULL;
	texture = IMG_LoadTexture(renderer, filename);
	if (texture == NULL) {
		logSDLError("LoadTexture");
	}
	return texture;
}

void renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y, int scale) {
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w * scale;
	rect.h = h  * scale;

	renderTextureRect(renderer, texture, &rect);
}

void renderTextureRect(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect *rect) {
	if (SDL_RenderCopy(renderer, texture, NULL, rect)) {
		logSDLError("renderTextureRect");
	}
}

void renderTextureTile(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect *tile, int x, int y, int scale) {
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	SDL_Rect destination;
	destination.x = x;
	destination.y = y;
	destination.w = tile->w * scale;
	destination.h = tile->h * scale;

	if (SDL_RenderCopy(renderer, texture, tile, &destination)) {
		logSDLError("renderTextureTile");
	}
}
