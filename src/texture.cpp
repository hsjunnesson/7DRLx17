#include <SDL.h>
#include <SDL_image.h>
#include <memory.h>
#include <fstream>

#include "json.hpp"

#include "texture.h"
#include "log.h"

namespace texture {
	using namespace foundation;
	using json = nlohmann::json;

    Atlas *create_atlas(Allocator &allocator, SDL_Renderer *renderer, const char *config_filename) {
		Atlas *atlas = MAKE_NEW(allocator, Atlas);
		if (!atlas) {
			log_fatal("Could not allocate Atlas");
		}

		json json_data;
		const char *texture_filename = nullptr;
		int tile_size = 0;
		int gutter = 0;

		try {
			std::ifstream input_file_stream(config_filename);			
			input_file_stream >> json_data;

			json::string_t *texture_filename_ptr = json_data["texture"].get_ptr<json::string_t *>();
			if (!texture_filename_ptr) {
				log_fatal("Could not read 'texture' from %s", config_filename);
			}

			texture_filename = texture_filename_ptr->c_str();

			tile_size = json_data["tile_size"].get<int>();
			gutter = json_data["gutter"].get<int>();
		} catch (const std::exception &e) {
			log_fatal("Could not parse json config file: %s", e.what());
		}

		SDL_Texture *texture = load(renderer, texture_filename);
		int w, h;
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

		atlas->w = w;
		atlas->h = h;
		atlas->tile_size = tile_size;
		atlas->gutter = gutter;
		atlas->w_tiles = w / tile_size;
		atlas->h_tiles = h / tile_size;
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
