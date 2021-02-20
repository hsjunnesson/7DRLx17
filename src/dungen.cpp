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
    int32_t from_room_index;
    int32_t to_room_index;
};

int dungen_thread(void *data) {
    World *world = (World *)data;
    if (!world) {
        log_fatal("No World in dungen_thread");
    }

    engine::DunGenParams params;
    config::read("assets/dungen_params.json", &params);

    TempAllocator1024 allocator;

    Hash<Tile> tiles = Hash<Tile>(allocator);

    int32_t map_width = params.map_width();

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
    std::uniform_int_distribution<int> percentage(1, 100);

    int32_t start_room_index = 0;
    int32_t boss_room_index = 0;

    bool start_room_vertical_side = fifty_fifty(random_engine) == 0;
    bool start_room_first_side = fifty_fifty(random_engine) == 0;

    if (start_room_vertical_side) {
        std::uniform_int_distribution<int32_t> tall_side_distribution(0, rooms_count_tall - 1);
        int32_t y = tall_side_distribution(random_engine);
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
        int32_t x = wide_side_distribution(random_engine);
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

    // Rooms and corridors collections
    Hash<GenRoom> rooms = Hash<GenRoom>(allocator);
    Array<Corridor> corridors = Array<Corridor>(allocator);

    // Place rooms in grids sections, referenced by their index.
    {
        for (int32_t room_index = 0; room_index < params.room_count(); ++room_index) {
            int32_t room_index_x, room_index_y;
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
    }

    // Place corridors
    {
        int32_t start_room_x, start_room_y;
        coord(start_room_index, start_room_x, start_room_y, rooms_count_wide);

        int32_t boss_room_x, boss_room_y;
        coord(boss_room_index, boss_room_x, boss_room_y, rooms_count_wide);

        Array<line::Coordinate> shortest_line_path = line::zig_zag(allocator, {(int32_t)start_room_x, (int32_t)start_room_y}, {(int32_t)boss_room_x, (int32_t)boss_room_y});

        for (int32_t i = 0; i < (int32_t)array::size(shortest_line_path) - 1; ++i) {
            line::Coordinate from = shortest_line_path[i];
            line::Coordinate to = shortest_line_path[i + 1];

            int32_t from_room_index = index(from.x, from.y, rooms_count_wide);
            int32_t to_room_index = index(to.x, to.y, rooms_count_wide);

            array::push_back(corridors, Corridor{from_room_index, to_room_index});
        }
    }

    // Expand some branches
    {
        Array<Corridor> branches = Array<Corridor>(allocator);

        // Returns a corridor to a random adjacent room, which isn't already connected to this room.
        auto expand = [&](int32_t room_index) {
            int32_t room_x, room_y;
            coord(room_index, room_x, room_y, rooms_count_wide);

            Array<int32_t> available_adjacent_room_indices = Array<int32_t>(allocator);

            auto adjacent_coordinates = {
                line::Coordinate{room_x + 1, room_y},
                line::Coordinate{room_x, room_y + 1},
                line::Coordinate{room_x - 1, room_y},
                line::Coordinate{room_x, room_y - 1}};

            // For each orthogonally adjacent room. Check if it's a valid location.
            for (line::Coordinate next_coordinate : adjacent_coordinates) {
                if (next_coordinate.x >= 0 &&
                    next_coordinate.x < rooms_count_wide &&
                    next_coordinate.y >= 0 &&
                    next_coordinate.y < rooms_count_tall) {
                    int32_t next_room_index = index(next_coordinate.x, next_coordinate.y, rooms_count_wide);
                    bool found = false;

                    // Check to make sure no other corridor exists that connect these two rooms.
                    for (auto iter = array::begin(corridors); iter != array::end(corridors); ++iter) {
                        if ((iter->from_room_index == room_index && iter->to_room_index == next_room_index) ||
                            (iter->to_room_index == room_index && iter->from_room_index == next_room_index)) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        array::push_back(available_adjacent_room_indices, next_room_index);
                    }
                }
            }

            if (array::size(available_adjacent_room_indices) > 0) {
                std::uniform_int_distribution<int> random_distribution(0, array::size(available_adjacent_room_indices) - 1);
                int random_selection = random_distribution(random_engine);
                int32_t next_room_index = available_adjacent_room_indices[random_selection];
                Corridor branching_corridor = Corridor{room_index, next_room_index};
                array::push_back(corridors, branching_corridor);
                return next_room_index;
            }

            return -1;
        };

        for (auto iter = array::begin(corridors); iter != array::end(corridors); ++iter) {
            // Don't branch from start or end room
            if (iter == array::begin(corridors)) {
                continue;
            }

            Corridor corridor = *iter;
            int32_t room_index = corridor.from_room_index;

            // Expand branches, perhaps multiple times
            bool expansion_done = percentage(random_engine) > params.expand_chance();
            while (!expansion_done) {
                int32_t next_room_index = expand(room_index);
                if (next_room_index >= 0) {
                    room_index = next_room_index;
                    expansion_done = percentage(random_engine) > params.expand_chance();
                } else {
                    expansion_done = true;
                }
            }
        }

        for (auto iter = array::begin(branches); iter != array::end(branches); ++iter) {
            Corridor branch = *iter;
            array::push_back(corridors, branch);
        }
    }

    // Prune disconnected rooms
    {
        Queue<int32_t> disconnected_room_indices = Queue<int32_t>(allocator);
        for (int32_t i = 0; i < params.room_count(); ++i) {
            bool found = false;

            for (auto iter = array::begin(corridors); iter != array::end(corridors); ++iter) {
                Corridor corridor = *iter;
                if (corridor.from_room_index == i || corridor.to_room_index == i) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                queue::push_front(disconnected_room_indices, (int32_t)i);
            }
        }

        for (int32_t i = 0; i < (int32_t)queue::size(disconnected_room_indices); ++i) {
            int32_t index = disconnected_room_indices[i];
            hash::remove(rooms, index);
        }
    }

    // Draw rooms as tiles
    {
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
    }

    // Draw corridors as tiles
    {
        int32_t floor_tile = hash::get(world->atlas.tiles_by_name, tile::Floor, 0);
        // int32_t corner_top_left_tile = hash::get(world->atlas.tiles_by_name, tile::WallCornerTopLeft, 0);
        int32_t horizontal_tile = hash::get(world->atlas.tiles_by_name, tile::WallHorizontal, 0);
        // int32_t corner_top_right_tile = hash::get(world->atlas.tiles_by_name, tile::WallCornerTopRight, 0);
        int32_t left_tile = hash::get(world->atlas.tiles_by_name, tile::WallLeft, 0);
        int32_t right_tile = hash::get(world->atlas.tiles_by_name, tile::WallRight, 0);
        // int32_t corner_bottom_left_tile = hash::get(world->atlas.tiles_by_name, tile::WallCornerBottomLeft, 0);
        // int32_t corner_bottom_right_tile = hash::get(world->atlas.tiles_by_name, tile::WallCornerBottomRight, 0);

        auto place_floor = [&](line::Coordinate prev, line::Coordinate coord, line::Coordinate next) {
            hash::set(tiles, index(coord.x, coord.y, map_width), {floor_tile});

            // Valid next and prev
            if (prev.x >= 0 && next.x >= 0) {
                if (prev.x != next.x && prev.y == next.y) { // Horizontal line
                    int32_t above = index(coord.x, coord.y - 1, map_width);
                    int32_t below = index(coord.x, coord.y + 1, map_width);

                    if (!hash::has(tiles, above)) {
                        hash::set(tiles, above, {horizontal_tile});
                    }

                    if (!hash::has(tiles, below)) {
                        hash::set(tiles, below, {horizontal_tile});
                    }

                    // TODO Change into a corner wall
                } else if (prev.x == next.x && prev.y != next.y) { // Vertical line
                    int32_t left = index(coord.x - 1, coord.y, map_width);
                    int32_t right = index(coord.x + 1, coord.y, map_width);

                    if (!hash::has(tiles, left)) {
                        hash::set(tiles, left, {left_tile});
                    }

                    if (!hash::has(tiles, right)) {
                        hash::set(tiles, right, {right_tile});
                    }
                }

                // TODO corners
            }
        };

        for (auto iter = array::begin(corridors); iter != array::end(corridors); ++iter) {
            Corridor corridor = *iter;

            GenRoom start_room = hash::get(rooms, corridor.from_room_index, {});
            GenRoom to_room = hash::get(rooms, corridor.to_room_index, {});

            line::Coordinate a = {start_room.x + start_room.w / 2, start_room.y + start_room.h / 2};
            line::Coordinate b = {to_room.x + to_room.w / 2, to_room.y + to_room.h / 2};

            Array<line::Coordinate> coordinates = line::zig_zag(allocator, a, b);

            for (int32_t line_i = 0; line_i < (int32_t)array::size(coordinates); ++line_i) {
                line::Coordinate prev;
                if (line_i > 0) {
                    prev = coordinates[line_i - 1];
                } else {
                    prev.x = -1;
                    prev.y = -1;
                }

                line::Coordinate coord = coordinates[line_i];

                line::Coordinate next;
                if (line_i < (int32_t)array::size(coordinates) - 1) {
                    next = coordinates[line_i + 1];
                } else {
                    next.x = -1;
                    next.y = -1;
                }

                place_floor(prev, coord, next);
            }
        }
    }

    // Update world's tiles.
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
