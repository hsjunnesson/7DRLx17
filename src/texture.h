#pragma once

#pragma warning(push, 0)
#include <SDL2/SDL.h>
#include "memory.h"
#include "hash.h"
#pragma warning(pop)

namespace texture {
    using namespace foundation;

    /**
     * @brief A tiled texture atlas
     * 
     */
    struct Atlas {
        Atlas(Allocator &allocator, SDL_Renderer *renderer, const char *param_filename);
        ~Atlas();

        Allocator &allocator;
        int w, h;
        int tile_size;
        int gutter;
        int w_tiles, h_tiles;
        Hash<int32_t> tiles_by_name;
        SDL_Texture *texture;
    };

    /**
     * @brief Loads a texture from a filename.
     *
     * @param sdl_renderer The SDL Renderer
     * @param filename The path to the texture file
     * @return SDL_Texture*
     */
    SDL_Texture *load(SDL_Renderer *renderer, const char *filename);
}
