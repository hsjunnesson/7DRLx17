#include <cstdlib>
#include <functional>

#include "dungen.h"
#include "game.h"
#include "log.h"
#include "texture.h"

namespace game {

namespace tile {

uint64_t hash(const char *s) {
    return murmur_hash_64(s, (uint32_t)strlen(s), 0);
};

} // namespace tile

Game::Game(Allocator &allocator, SDL_Renderer *renderer, const char *atlas_config_filename)
: allocator(allocator)
, game_state(GameState::None)
, mutex(SDL_CreateMutex())
, dungen_thread(nullptr)
, atlas(texture::Atlas(allocator, renderer, atlas_config_filename))
, x_offset(0)
, y_offset(0)
, zoom_level(1)
, terrain_tiles(Hash<Tile>(allocator))
, entity_tiles(Hash<Tile>(allocator))
, max_width(0) {
    if (!hash::has(atlas.tiles_by_name, tile::Missing)) {
        log_fatal("Atlas does not have the 'missing' named tile.");
    }
}

Game::~Game() {
    if (dungen_thread) {
        SDL_DetachThread(dungen_thread);
    }

    if (mutex) {
        SDL_DestroyMutex(mutex);
    }
}

void update(Game &game, uint32_t t, double dt) {
    (void)t;
    (void)dt;

    switch (game.game_state) {
    case GameState::None: {
        transition(game, GameState::Initializing);
        break;
    }
    case GameState::Playing: {
        break;
    }
    case GameState::Quitting: {
        transition(game, GameState::Terminate);
        break;
    }
    default:
        break;
    }
}

void on_input(Game &game, input::InputCommand input_command) {
    switch (input_command.action) {
    case input::Action::Quit: {
        if (game.game_state == GameState::Playing) {
            transition(game, GameState::Quitting);
        }
        break;
    }
    case input::Action::ZoomIn: {
        if (game.zoom_level < 4) {
            ++game.zoom_level;
        }
        break;
    }
    case input::Action::ZoomOut: {
        if (game.zoom_level > 1) {
            --game.zoom_level;
        }
        break;
    }
    case input::Action::Mouse: {
        if (input_command.mouse_state.mouse_left_state == input::MouseButtonState::Repeated) {
            game.x_offset += input_command.mouse_state.mouse_relative_motion.x;
            game.y_offset += input_command.mouse_state.mouse_relative_motion.y;
        }
        break;
    }
    }
}

void render(Game &game, SDL_Renderer *renderer) {
    if (game.game_state != GameState::Playing) {
        return;
    }

    if (SDL_TryLockMutex(game.mutex) != 0) {
        return;
    }

    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    int tile_size = game.atlas.tile_size;
    int gutter = game.atlas.gutter;

    std::function draw_tile = [&](uint64_t pos_index, Tile tile) {
        int tile_index = tile.index;

        SDL_Rect source;
        int32_t source_x, source_y;
        coord(tile_index, source_x, source_y, (int32_t)game.atlas.w_tiles - 1);
        source.x = (int)(source_x * tile_size + source_x * gutter);
        source.y = (int)(source_y * tile_size + source_y * gutter);
        source.w = tile_size;
        source.h = tile_size;

        SDL_Rect destination;
        int32_t destination_x, destination_y;
        coord((int32_t)pos_index, destination_x, destination_y, game.max_width);
        destination.x = (int)(destination_x * tile_size * game.zoom_level) + game.x_offset;
        destination.y = (int)(destination_y * tile_size * game.zoom_level) + game.y_offset;
        destination.w = tile_size * game.zoom_level;
        destination.h = tile_size * game.zoom_level;

        SDL_RenderCopyEx(renderer, game.atlas.texture, &source, &destination, tile.angle, nullptr, tile.flip);
    };

    for (const Hash<Tile>::Entry *it = hash::begin(game.terrain_tiles); it != hash::end(game.terrain_tiles); ++it) {
        uint64_t pos_index = it->key;
        Tile tile = it->value;
        draw_tile(pos_index, tile);
    }

    for (const Hash<Tile>::Entry *it = hash::begin(game.entity_tiles); it != hash::end(game.entity_tiles); ++it) {
        uint64_t pos_index = it->key;
        Tile tile = it->value;
        draw_tile(pos_index, tile);
    }

    SDL_UnlockMutex(game.mutex);
}

void transition(Game &game, GameState game_state) {
    if (SDL_LockMutex(game.mutex) != 0) {
        log_fatal("Could not lock mutex %s", SDL_GetError());
    }

    // When leaving a game state
    switch (game.game_state) {
    case GameState::DunGen:
        if (game.dungen_thread) {
            SDL_DetachThread(game.dungen_thread);
            game.dungen_thread = nullptr;
        }
        break;
    default:
        break;
    }

    game.game_state = game_state;

    // When entering a new game state
    switch (game.game_state) {
    case GameState::None: {
        break;
    }
    case GameState::Initializing: {
        log_info("Initializing");
        transition(game, GameState::DunGen);
        break;
    }
    case GameState::DunGen: {
        log_info("DunGen started");
        SDL_Thread *threadID = SDL_CreateThread(dungen_thread, "dungen", &game);
        game.dungen_thread = threadID;
        break;
    }
    case GameState::Playing: {
        log_info("Playing");
        break;
    }
    case GameState::Quitting: {
        log_info("Quitting");
        break;
    }
    case GameState::Terminate: {
        log_info("Terminating");
        break;
    }
    }

    SDL_UnlockMutex(game.mutex);
}

} // namespace game
