#include <SDL.h>

#pragma once

namespace foundation {
    class Allocator;
}

namespace texture {
    using namespace foundation;

    /**
     * @brief A tiled texture atlas
     * 
     */
    struct Atlas {
        int w, h;
        int tile_size;
        int gutter;
        SDL_Texture *texture;
    };

    /**
     * @brief Create an atlas object.
     * 
     * @param allocator The allocator.
     * @param sdl_renderer The SDL renderer.
     * @param config_filename The path to the json config file/
     * @return Atlas* 
     */
    Atlas *create_atlas(Allocator &allocator, SDL_Renderer *renderer, const char *config_filename);

    /**
     * @brief Destroys an atlas object.
     * 
     * @param allocator The allocator.
     * @param atlas The atlas to destroy.
     */
    void destroy_atlas(Allocator &allocator, Atlas *atlas);

    /**
     * @brief Loads a texture from a filename.
     *
     * @param sdl_renderer The SDL Renderer
     * @param filename The path to the texture file
     * @return SDL_Texture*
     */
    SDL_Texture *load(SDL_Renderer *renderer, const char *filename);
}
