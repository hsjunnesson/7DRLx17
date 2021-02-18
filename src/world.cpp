#include <cstdlib>

#include "dungen.h"
#include "log.h"
#include "texture.h"
#include "world.h"

namespace world {

namespace tile {

uint64_t hash(const char *s) {
    return murmur_hash_64(s, (uint32_t)strlen(s), 0);
};

} // namespace tile

World::World(Allocator &allocator, SDL_Renderer *renderer, const char *atlas_config_filename)
: allocator(allocator)
, game_state(GameState::None)
, mutex(SDL_CreateMutex())
, dungen_thread(nullptr)
, atlas(texture::Atlas(allocator, renderer, atlas_config_filename))
, x_offset(0)
, y_offset(0)
, tiles(Hash<Tile>(allocator))
, max_width(0) {
    if (!hash::has(atlas.tiles_by_name, tile::Missing)) {
        log_fatal("Atlas does not have the 'missing' named tile.");
    }
}

World::~World() {
    if (dungen_thread) {
        SDL_DetachThread(dungen_thread);
    }

    if (mutex) {
        SDL_DestroyMutex(mutex);
    }
}

void update(World &world, uint32_t t, double dt) {
    (void)t;
    (void)dt;

    switch (world.game_state) {
    case GameState::None:
        transition(world, GameState::Initializing);
        break;
    case GameState::Playing:
        break;
    default:
        break;
    }
}

void render(World &world, SDL_Renderer *renderer) {
    if (world.game_state != GameState::Playing) {
        return;
    }

    if (SDL_TryLockMutex(world.mutex) != 0) {
        return;
    }

    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    int tile_size = world.atlas.tile_size;
    int gutter = world.atlas.gutter;

    for (const Hash<Tile>::Entry *it = hash::begin(world.tiles); it != hash::end(world.tiles); ++it) {
        uint64_t pos_index = it->key;
        Tile tile = it->value;
        int tile_index = tile.index;

        if (tile_index == tile::Floor) {
            continue;
        }

        SDL_Rect source;
        int32_t source_x, source_y;
        coord(tile_index, source_x, source_y, (int32_t)world.atlas.w_tiles - 1);
        source.x = (int)(source_x * tile_size + source_x * gutter);
        source.y = (int)(source_y * tile_size + source_y * gutter);
        source.w = tile_size;
        source.h = tile_size;

        SDL_Rect destination;
        int32_t destination_x, destination_y;
        coord((int32_t)pos_index, destination_x, destination_y, world.max_width);
        destination.x = (int)(destination_x * tile_size) + world.x_offset;
        destination.y = (int)(destination_y * tile_size) + world.y_offset;
        destination.w = tile_size;
        destination.h = tile_size;

        SDL_RenderCopyEx(renderer, world.atlas.texture, &source, &destination, tile.angle, nullptr, tile.flip);
    }

    SDL_UnlockMutex(world.mutex);
}

void transition(World &world, GameState game_state) {
    if (SDL_LockMutex(world.mutex) != 0) {
        log_fatal("Could not lock mutex %s", SDL_GetError());
    }

    // When leaving a game state
    switch (world.game_state) {
    case GameState::DunGen:
        if (world.dungen_thread) {
            SDL_DetachThread(world.dungen_thread);
            world.dungen_thread = nullptr;
        }
        break;
    default:
        break;
    }

    world.game_state = game_state;

    // When entering a new game state
    switch (game_state) {
    case GameState::None:
        break;
    case GameState::Initializing: {
        log_info("Initializing");
        transition(world, GameState::DunGen);
        break;
    }
    case GameState::DunGen: {
        log_info("DunGen started");
        SDL_Thread *threadID = SDL_CreateThread(dungen_thread, "dungen", &world);
        world.dungen_thread = threadID;
        break;
    }
    case GameState::Playing:
        log_info("Playing");
        break;
    }

    SDL_UnlockMutex(world.mutex);
}

} // namespace world
