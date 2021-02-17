#include "dungen.h"

#include <random>

#pragma warning(push, 0)
#include "array.h"
#include "hash.h"
#include "memory.h"
#include "proto/engine.pb.h"
#pragma warning(pop)

#include "config.h"
#include "log.h"
#include "world.h"

namespace world {
using namespace foundation;

struct RoomRect {
    uint64_t x, y, w, h;
};

int dungen_thread(void *data) {
    World *world = (World *)data;
    if (!world) {
        log_fatal("No World in dungen_thread");
    }

    engine::DunGenParams params;
    config::read("assets/dungen_params.json", &params);

    Allocator &allocator = memory_globals::default_allocator();

    Hash<Tile> tiles = Hash<Tile>(allocator);
    uint64_t map_width = params.map_width();

    const int64_t rooms_count_wide = (int64_t)ceil(sqrt(params.room_count()));
    const int64_t rooms_count_tall = (int64_t)ceil(sqrt(params.room_count()));
    const int64_t section_width = params.map_width() / rooms_count_wide;
    const int64_t section_height = params.map_height() / rooms_count_tall;

    std::random_device random_device;
    std::mt19937 random_engine(random_device());
    unsigned int seed = (unsigned int)time(nullptr);
    random_engine.seed(seed);

    log_debug("Dungen seeded with %u", seed);

    std::uniform_int_distribution<int64_t> room_size_distribution(params.min_room_size(), params.max_room_size());
    std::uniform_int_distribution<int> fifty_fifty(0, 1);

    int64_t start_room_index = 0;
    int64_t boss_room_index = 0;

    bool start_room_vertical_side = fifty_fifty(random_engine) == 0;
    bool start_room_first_side = fifty_fifty(random_engine) == 0;

    if (start_room_vertical_side) {
        std::uniform_int_distribution<int64_t> tall_side_distribution(0, rooms_count_tall - 1);
        uint64_t y = tall_side_distribution(random_engine);
        start_room_index = y * rooms_count_wide;

        if (!start_room_first_side) {
            start_room_index += (rooms_count_wide - 1);
        }

        y = tall_side_distribution(random_engine);
        boss_room_index = y * rooms_count_wide;

        if (start_room_first_side) {
            boss_room_index += (rooms_count_wide - 1);
        }
    } else {
        std::uniform_int_distribution<int64_t> wide_side_distribution(0, rooms_count_wide - 1);
        uint64_t x = wide_side_distribution(random_engine);
        start_room_index = x;

        if (!start_room_first_side) {
            start_room_index += rooms_count_wide * (rooms_count_tall - 1);
        }

        x = wide_side_distribution(random_engine);
        boss_room_index = x;

        if (start_room_first_side) {
            start_room_index += rooms_count_wide * (rooms_count_tall - 1);
        }
    }

    if (start_room_index > params.room_count()) {
        start_room_index = params.room_count() - 1;
    }

    if (boss_room_index > params.room_count()) {
        boss_room_index = params.room_count() - 1;
    }

    log_debug("Dungen start room index %d", start_room_index);
    log_debug("Dungen boss room index %d", boss_room_index);

    Array<RoomRect> rooms = Array<RoomRect>(allocator);

    for (int room_index = 0; room_index < params.room_count(); ++room_index) {
        uint64_t room_index_x, room_index_y;
        coord(room_index, room_index_x, room_index_y, rooms_count_wide);

        const uint64_t section_min_x = room_index_x * section_width;
        const uint64_t section_max_x = section_min_x + section_width;
        const uint64_t section_min_y = room_index_y * section_height;
        const uint64_t section_max_y = section_min_y + section_height;

        const uint64_t room_width = room_size_distribution(random_engine);
        const uint64_t room_height = room_size_distribution(random_engine);

        const uint64_t room_x = section_min_x + section_width / 2 - room_width / 2;
        const uint64_t room_y = section_min_y + section_height / 2 - room_height / 2;

        array::push_back(rooms, RoomRect{room_x, room_y, room_width, room_height});
    }

    int room_index = 0;
    for (auto iter = array::begin(rooms); iter != array::end(rooms); ++iter) {
        RoomRect room = *iter;

        for (int y = 0; y < room.h; ++y) {
            for (int x = 0; x < room.w; ++x) {
                int32_t tile_index = 0;
                int32_t floor_tile = hash::get(world->atlas.tiles_by_name, tile::Floor, 0);
                
                if (room_index == start_room_index) {
                    floor_tile = hash::get(world->atlas.tiles_by_name, tile::Snake, 0);
                } else if (room_index == boss_room_index) {
                    floor_tile = hash::get(world->atlas.tiles_by_name, tile::Ghost, 0);
                }

                if (y == 0 ) {
                    if (x == 0) {
                        tile_index = hash::get(world->atlas.tiles_by_name, tile::WallCornerTopLeft, 0);
                    } else if (x == room.w - 1) {
                        tile_index = hash::get(world->atlas.tiles_by_name, tile::WallCornerTopRight, 0);
                    } else {
                        tile_index = hash::get(world->atlas.tiles_by_name, tile::WallHorizontal, 0);
                    }
                } else if (y == room.h - 1) {
                    if (x == 0) {
                        tile_index = hash::get(world->atlas.tiles_by_name, tile::WallCornerBottomLeft, 0);
                    } else if (x == room.w - 1) {
                        tile_index = hash::get(world->atlas.tiles_by_name, tile::WallCornerBottomRight, 0);
                    } else {
                        tile_index = hash::get(world->atlas.tiles_by_name, tile::WallHorizontal, 0);
                    }
                } else if (x == 0) {
                    tile_index = hash::get(world->atlas.tiles_by_name, tile::WallLeft, 0);
                } else if (x == room.w - 1) {
                    tile_index = hash::get(world->atlas.tiles_by_name, tile::WallRight, 0);
                } else {
                    tile_index = floor_tile;
                }

                hash::set(tiles, index(room.x + x, room.y + y, map_width), {tile_index});
            }
        }

        ++room_index;
    }

    if (SDL_LockMutex(world->mutex) == 0) {
        world->tiles = tiles;
        world->max_width = map_width;
        SDL_UnlockMutex(world->mutex);
    } else {
        log_fatal("Could not lock mutex %s", SDL_GetError());
    }

    log_info("DunGen completed");

    transition(*world, GameState::Playing);

    return 0;
}

} // namespace world
