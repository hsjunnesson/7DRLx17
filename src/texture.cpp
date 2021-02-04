#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <memory.h>
#include <fstream>

#include "proto/engine.pb.h"

#include "texture.h"
#include "log.h"
#include "config.h"

namespace texture {
	using namespace foundation;

    Atlas *create_atlas(Allocator &allocator, SDL_Renderer *renderer, const char *param_filename) {
		Atlas *atlas = MAKE_NEW(allocator, Atlas);
		if (!atlas) {
			log_fatal("Could not allocate Atlas");
		}

		engine::AtlasParams params;

		config::read(param_filename, &params);

		SDL_Texture *texture = load(renderer, params.texture().c_str());
		int w, h;
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

		SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
		
		atlas->w = w;
		atlas->h = h;
		atlas->tile_size = params.tile_size();
		atlas->gutter = params.gutter();
		atlas->w_tiles = w / params.tile_size();
		atlas->h_tiles = h / params.tile_size();
		atlas->texture = texture;

		return atlas;
	}

	void destroy_atlas(Allocator &allocator, Atlas *atlas) {
		if (atlas) {
			if (atlas->texture) {
				SDL_DestroyTexture(atlas->texture);
			}

			allocator.deallocate(atlas);
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
}
