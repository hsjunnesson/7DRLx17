#include <SDL.h>

#pragma once

namespace texture {
    /**
     * @brief Loads a texture from a filename.
     *
     * @param renderer
     * @param filename
     * @return SDL_Texture*
     */
    SDL_Texture* load_texture(SDL_Renderer *renderer, const char *filename);

    /**
     * @brief Renders a texture at a point.
     *
     * @param renderer
     * @param texture
     * @param x
     * @param y
     * @param scale
     */
    void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y, int scale = 1);

    /**
     * @brief Renders a texture in a rect.
     *
     * @param renderer
     * @param texture
     * @param rect
     */
    void render_texture_rect(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect *rect);

    /**
     * @brief Renders a texture tile at a point.
     *
     * @param renderer
     * @param texture
     * @param tile
     * @param x
     * @param y
     * @param scale
     */
    void render_texture_tile(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect *tile, int x, int y, int scale = 1);
}
