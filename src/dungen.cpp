#include "dungen.h"

#include <random>

#pragma warning(push, 0)
#include "array.h"
#include "hash.h"
#include "memory.h"
#include "proto/engine.pb.h"
#include "queue.h"
#pragma warning(pop)

#include "config.h"
#include "line.hpp"
#include "log.h"
#include "world.h"

namespace world {
using namespace foundation;

struct GenRoom {
    int32_t x, y;
    int32_t w, h;
    bool start_room = false;
    bool boss_room = false;
};

struct Corridor {
    uint32_t from_room_index;
    uint32_t to_room_index;
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
    uint32_t map_width = params.map_width();

    const int32_t rooms_count_wide = (int32_t)ceil(sqrt(params.room_count()));
    const int32_t rooms_count_tall = (int32_t)ceil(sqrt(params.room_count()));
    const int32_t section_width = params.map_width() / rooms_count_wide;
    const int32_t section_height = params.map_height() / rooms_count_tall;

    log_debug("Dungen rooms count wide %u", rooms_count_wide);
    log_debug("Dungen rooms count tall %u", rooms_count_tall);
    log_debug("Dungen section width %u", section_width);
    log_debug("Dungen section height %u", section_height);

    std::random_device random_device;
    std::mt19937 random_engine(random_device());
    unsigned int seed = (unsigned int)time(nullptr);
    random_engine.seed(seed);

    log_debug("Dungen seeded with %u", seed);

    std::uniform_int_distribution<int32_t> room_size_distribution(params.min_room_size(), params.max_room_size());
    std::uniform_int_distribution<int> fifty_fifty(0, 1);

    int32_t start_room_index = 0;
    int32_t boss_room_index = 0;

    bool start_room_vertical_side = fifty_fifty(random_engine) == 0;
    bool start_room_first_side = fifty_fifty(random_engine) == 0;

    if (start_room_vertical_side) {
        std::uniform_int_distribution<int32_t> tall_side_distribution(0, rooms_count_tall - 1);
        uint32_t y = tall_side_distribution(random_engine);
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
        std::uniform_int_distribution<int32_t> wide_side_distribution(0, rooms_count_wide - 1);
        uint32_t x = wide_side_distribution(random_engine);
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

    // Place rooms in grids sections, referenced by their index.
    Hash<GenRoom> rooms = Hash<GenRoom>(allocator);

    for (int room_index = 0; room_index < params.room_count(); ++room_index) {
        uint32_t room_index_x, room_index_y;
        coord(room_index, room_index_x, room_index_y, rooms_count_wide);

        const int32_t section_min_x = room_index_x * section_width;
        const int32_t section_max_x = section_min_x + section_width;
        const int32_t section_min_y = room_index_y * section_height;
        const int32_t section_max_y = section_min_y + section_height;

        const int32_t room_width = room_size_distribution(random_engine);
        const int32_t room_height = room_size_distribution(random_engine);

        std::uniform_int_distribution<int32_t> x_offset(section_min_x + 1, section_max_x - 1 - room_width);
        std::uniform_int_distribution<int32_t> y_offset(section_min_y + 1, section_max_y - 1 - room_height);

        const int32_t room_x = x_offset(random_engine);
        const int32_t room_y = y_offset(random_engine);

        GenRoom room = GenRoom{room_x, room_y, room_width, room_height};

        if (room_index == start_room_index) {
            room.start_room = true;
        }

        if (room_index == boss_room_index) {
            room.boss_room = true;
        }

        hash::set(rooms, room_index, room);
    }

    // Place corridors
    Array<Corridor> corridors = Array<Corridor>(allocator);

    uint32_t start_room_x, start_room_y;
    coord(start_room_index, start_room_x, start_room_y, rooms_count_wide);

    uint32_t boss_room_x, boss_room_y;
    coord(boss_room_index, boss_room_x, boss_room_y, rooms_count_wide);

    TempAllocator64 ta;
    Array<line::Coordinate> shortest_line_path = line::zig_zag(ta, {(int32_t)start_room_x, (int32_t)start_room_y}, {(int32_t)boss_room_x, (int32_t)boss_room_y});

    for (uint32_t i = 0; i < array::size(shortest_line_path) - 1; ++i) {
        line::Coordinate from = shortest_line_path[i];
        line::Coordinate to = shortest_line_path[i+1];

        uint32_t from_room_index = index(from.x, from.y, rooms_count_wide);
        uint32_t to_room_index = index(to.x, to.y, rooms_count_wide);

        array::push_back(corridors, Corridor{from_room_index, to_room_index});
    }

    // Prune disconnected rooms
    Queue<uint32_t> disconnected_room_indices = Queue<uint32_t>(ta);
    for (uint32_t i = 0; i < (uint32_t)params.room_count(); ++i) {
        bool found = false;

        for (auto iter = array::begin(corridors); iter != array::end(corridors); ++iter) {
            Corridor corridor = *iter;
            if (corridor.from_room_index == i || corridor.to_room_index == i) {
                found = true;
                break;
            }
        }

        if (!found) {
            queue::push_front(disconnected_room_indices, i);
        }
    }

    for (uint32_t i = 0; i < queue::size(disconnected_room_indices); ++i) {
        uint32_t index = disconnected_room_indices[i];
        hash::remove(rooms, index);
    }

    // Draw rooms as tiles
    for (auto iter = hash::begin(rooms); iter != hash::end(rooms); ++iter) {
        GenRoom room = iter->value;

        for (int y = 0; y < room.h; ++y) {
            for (int x = 0; x < room.w; ++x) {
                int32_t tile_index = 0;
                int32_t floor_tile = hash::get(world->atlas.tiles_by_name, tile::Floor, 0);

                if (room.start_room) {
                    floor_tile = hash::get(world->atlas.tiles_by_name, tile::Snake, 0);
                } else if (room.boss_room) {
                    floor_tile = hash::get(world->atlas.tiles_by_name, tile::Ghost, 0);
                }

                if (y == 0) {
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
    }

    // Draw corridors as tiles
    for (auto iter = array::begin(corridors); iter != array::end(corridors); ++iter) {
        Corridor corridor = *iter;

        GenRoom start_room = hash::get(rooms, corridor.from_room_index, {});
        GenRoom to_room = hash::get(rooms, corridor.to_room_index, {});

        line::Coordinate a = {start_room.x + start_room.w / 2, start_room.y + start_room.h / 2};
        line::Coordinate b = {to_room.x + to_room.w / 2, to_room.y + to_room.h / 2};

        Array<line::Coordinate> coordinates = line::zig_zag(ta, a, b);

        for (auto inner_iter = array::begin(coordinates); inner_iter != array::end(coordinates); ++inner_iter) {
           line::Coordinate coordinate = *inner_iter;

           int32_t floor_tile = hash::get(world->atlas.tiles_by_name, tile::Ghost, 0);
           hash::set(tiles, index(coordinate.x, coordinate.y, map_width), {floor_tile});
        }
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
