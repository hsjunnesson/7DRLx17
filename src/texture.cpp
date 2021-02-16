#include <fstream>

#pragma warning(push, 0)
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <memory.h>
#include "proto/engine.pb.h"
#include "murmur_hash.h"
#pragma warning(pop)

#include "texture.h"
#include "log.h"
#include "config.h"

namespace texture {
	using namespace foundation;
	using namespace google;

	Atlas::Atlas(Allocator &allocator, SDL_Renderer *renderer, const char *param_filename)
	: allocator(allocator)
	, w(0)
	, h(0)
	, tile_size(0)
	, gutter(0)
	, w_tiles(0)
	, h_tiles(0)
	, tiles_by_name(Hash<int32_t>(allocator))
	, texture(nullptr)
	{
		engine::AtlasParams params;

		config::read(param_filename, &params);

		texture = load(renderer, params.texture().c_str());
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
		SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
		
		this->tile_size = params.tile_size();
		this->gutter = params.gutter();
		this->w_tiles = w / params.tile_size();
		this->h_tiles = h / params.tile_size();

		for (protobuf::Map<std::string, protobuf::int32>::const_iterator i = params.tile_names().begin(); i != params.tile_names().end(); ++i) {
			std::string key = i->first;
			protobuf::int32 value = i->second;
			const char *s = key.c_str();
			uint64_t hash = murmur_hash_64(s, (uint32_t)strlen(s), 0);
			hash::set(tiles_by_name, hash, value);
		}
	}

	Atlas::~Atlas() {
		if (texture) {
			SDL_DestroyTexture(texture);
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
