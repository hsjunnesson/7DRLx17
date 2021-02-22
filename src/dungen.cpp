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
#include "game.h"
#include "line.hpp"
#include "log.h"

namespace game {
using namespace foundation;

int dungen_thread(void *data) {
    Game *game = (Game *)data;
    if (!game) {
        log_fatal("No Game in dungen_thread");
    }

    engine::DunGenParams params;
    config::read("assets/dungen_params.json", &params);

    TempAllocator1024 ta;

    Hash<Tile> tiles = Hash<Tile>(game->allocator);

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

    // Rooms and corridors collections
    Hash<Room> rooms = Hash<Room>(game->allocator);
    Array<Corridor> corridors = Array<Corridor>(game->allocator);

    // Decide whether main orientation is vertical or horizontal
    {
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
    }

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

            std::uniform_int_distribution<int32_t> x_offset(section_min_x + 2, section_max_x - 2 - room_width);
            std::uniform_int_distribution<int32_t> y_offset(section_min_y + 2, section_max_y - 2 - room_height);

            const int32_t room_x = x_offset(random_engine);
            const int32_t room_y = y_offset(random_engine);

            Room room = Room{room_index, room_x, room_y, room_width, room_height};

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

        Array<line::Coordinate> shortest_line_path = line::zig_zag(ta, {(int32_t)start_room_x, (int32_t)start_room_y}, {(int32_t)boss_room_x, (int32_t)boss_room_y});

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
        Array<Corridor> branches = Array<Corridor>(ta);

        // Returns a corridor to a random adjacent room, which isn't already connected to this room.
        auto expand = [&](int32_t room_index) {
            int32_t room_x, room_y;
            coord(room_index, room_x, room_y, rooms_count_wide);

            Array<int32_t> available_adjacent_room_indices = Array<int32_t>(ta);

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
        Queue<int32_t> disconnected_room_indices = Queue<int32_t>(ta);
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
        const int32_t floor_tile = hash::get(game->atlas.tiles_by_name, tile::Floor, 0);

        for (auto iter = hash::begin(rooms); iter != hash::end(rooms); ++iter) {
            Room room = iter->value;

            for (int y = 0; y < room.h; ++y) {
                for (int x = 0; x < room.w; ++x) {
                    int32_t tile_index = 0;

                    if (y == 0) {
                        if (x == 0) {
                            tile_index = hash::get(game->atlas.tiles_by_name, tile::WallCornerTopLeft, 0);
                        } else if (x == room.w - 1) {
                            tile_index = hash::get(game->atlas.tiles_by_name, tile::WallCornerTopRight, 0);
                        } else {
                            tile_index = hash::get(game->atlas.tiles_by_name, tile::WallHorizontal, 0);
                        }
                    } else if (y == room.h - 1) {
                        if (x == 0) {
                            tile_index = hash::get(game->atlas.tiles_by_name, tile::WallCornerBottomLeft, 0);
                        } else if (x == room.w - 1) {
                            tile_index = hash::get(game->atlas.tiles_by_name, tile::WallCornerBottomRight, 0);
                        } else {
                            tile_index = hash::get(game->atlas.tiles_by_name, tile::WallHorizontal, 0);
                        }
                    } else if (x == 0) {
                        tile_index = hash::get(game->atlas.tiles_by_name, tile::WallLeft, 0);
                    } else if (x == room.w - 1) {
                        tile_index = hash::get(game->atlas.tiles_by_name, tile::WallRight, 0);
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
        const int32_t floor_tile = hash::get(game->atlas.tiles_by_name, tile::Floor, 0);
        const int32_t wall_corner_top_left_tile = hash::get(game->atlas.tiles_by_name, tile::WallCornerTopLeft, 0);
        const int32_t wall_horizontal_tile = hash::get(game->atlas.tiles_by_name, tile::WallHorizontal, 0);
        const int32_t wall_corner_top_right_tile = hash::get(game->atlas.tiles_by_name, tile::WallCornerTopRight, 0);
        const int32_t wall_left_tile = hash::get(game->atlas.tiles_by_name, tile::WallLeft, 0);
        const int32_t wall_right_tile = hash::get(game->atlas.tiles_by_name, tile::WallRight, 0);
        const int32_t wall_corner_bottom_left_tile = hash::get(game->atlas.tiles_by_name, tile::WallCornerBottomLeft, 0);
        const int32_t wall_corner_bottom_right_tile = hash::get(game->atlas.tiles_by_name, tile::WallCornerBottomRight, 0);
        const int32_t corridor_corner_up_right_tile = hash::get(game->atlas.tiles_by_name, tile::CorridorCornerUpRight, 0);
        const int32_t corridor_corner_up_left_tile = hash::get(game->atlas.tiles_by_name, tile::CorridorCornerUpLeft, 0);
        const int32_t corridor_corner_down_right_tile = hash::get(game->atlas.tiles_by_name, tile::CorridorCornerDownRight, 0);
        const int32_t corridor_corner_down_left_tile = hash::get(game->atlas.tiles_by_name, tile::CorridorCornerDownLeft, 0);

        const int32_t debug_tile = hash::get(game->atlas.tiles_by_name, tile::Missing, 0);

        // Added walls which needs to be properly placed.
        Hash<bool> placeholder_walls = Hash<bool>(ta);

        // Function to iterate through corridors, applying a function on each coordinate.
        auto iterate_corridor = [&](std::function<void(line::Coordinate prev, line::Coordinate coord, line::Coordinate next)> apply) {
            for (auto iter = array::begin(corridors); iter != array::end(corridors); ++iter) {
                Corridor corridor = *iter;

                Room start_room = hash::get(rooms, corridor.from_room_index, {});
                Room to_room = hash::get(rooms, corridor.to_room_index, {});

                line::Coordinate a = {start_room.x + start_room.w / 2, start_room.y + start_room.h / 2};
                line::Coordinate b = {to_room.x + to_room.w / 2, to_room.y + to_room.h / 2};

                Array<line::Coordinate> coordinates = line::zig_zag(ta, a, b);

                for (int32_t line_i = 0; line_i < (int32_t)array::size(coordinates); ++line_i) {
                    line::Coordinate prev;
                    if (line_i > 0) {
                        prev = coordinates[line_i - 1];
                    } else {
                        prev.x = -1;
                        prev.y = -1;
                    }

                    line::Coordinate coord = coordinates[line_i];

                    // Don't place corridors if we're inside the start or to room.
                    // But place corridor in the wall of the rooms.
                    bool inside_start_room =
                        coord.x > start_room.x &&
                        coord.y > start_room.y &&
                        coord.x < start_room.x + start_room.w - 1 &&
                        coord.y < start_room.y + start_room.h - 1;

                    bool inside_to_room =
                        coord.x > to_room.x &&
                        coord.y > to_room.y &&
                        coord.x < to_room.x + to_room.w - 1 &&
                        coord.y < to_room.y + to_room.h - 1;

                    if (inside_start_room || inside_to_room) {
                        continue;
                    }

                    line::Coordinate next;
                    if (line_i < (int32_t)array::size(coordinates) - 1) {
                        next = coordinates[line_i + 1];
                    } else {
                        next.x = -1;
                        next.y = -1;
                    }

                    apply(prev, coord, next);
                }
            }
        };

        // Dig out corridors
        iterate_corridor([&](line::Coordinate prev, line::Coordinate coord, line::Coordinate next) {
            (void)prev;
            (void)next;
            hash::set(tiles, index(coord.x, coord.y, map_width), {floor_tile});
        });

        // Place placeholder walls
        iterate_corridor([&](line::Coordinate prev, line::Coordinate coord, line::Coordinate next) {
            // Valid next and prev
            if (prev.x >= 0 && next.x >= 0) {
                if (prev.x != next.x && prev.y == next.y) { // Horizontal line
                    int32_t above = index(coord.x, coord.y - 1, map_width);
                    int32_t below = index(coord.x, coord.y + 1, map_width);

                    if (!hash::has(tiles, above) || hash::get(tiles, above, {}).index != floor_tile) {
                        hash::set(placeholder_walls, above, true);
                    }

                    if (!hash::has(tiles, below) || hash::get(tiles, below, {}).index != floor_tile) {
                        hash::set(placeholder_walls, below, true);
                    }
                } else if (prev.x == next.x && prev.y != next.y) { // Vertical line
                    int32_t left = index(coord.x - 1, coord.y, map_width);
                    int32_t right = index(coord.x + 1, coord.y, map_width);

                    if (!hash::has(tiles, left) || hash::get(tiles, left, {}).index != floor_tile) {
                        hash::set(placeholder_walls, left, true);
                    }

                    if (!hash::has(tiles, right) || hash::get(tiles, right, {}).index != floor_tile) {
                        hash::set(placeholder_walls, right, true);
                    }
                } else { // Corner
                    for (int y = -1; y <= 1; ++y) {
                        for (int x = -1; x <= 1; ++x) {
                            if (x == 0 && y == 0) {
                                continue;
                            }

                            if (coord.x - x == next.x && coord.y - y == next.y ||
                                coord.x - x == prev.x && coord.y - y == prev.y) {
                                continue;
                            }

                            int32_t adjacent_index = index(coord.x - x, coord.y - y, map_width);

                            if (!hash::has(tiles, adjacent_index) || hash::get(tiles, adjacent_index, {}).index != floor_tile) {
                                hash::set(placeholder_walls, adjacent_index, true);
                            }
                        }
                    }
                }
            }
        });

        // Function to update the correct wall tile on a coordinate. Depends on surrounding walls and placeholder walls.
        auto place_wall = [&](int32_t index_wall) {
            int32_t coord_x, coord_y;
            coord(index_wall, coord_x, coord_y, map_width);

            const int32_t index_nw = index(coord_x - 1, coord_y - 1, map_width);
            const int32_t index_n = index(coord_x, coord_y - 1, map_width);
            const int32_t index_ne = index(coord_x + 1, coord_y - 1, map_width);
            const int32_t index_w = index(coord_x - 1, coord_y, map_width);
            const int32_t index_e = index(coord_x + 1, coord_y, map_width);
            const int32_t index_sw = index(coord_x - 1, coord_y + 1, map_width);
            const int32_t index_s = index(coord_x, coord_y + 1, map_width);
            const int32_t index_se = index(coord_x + 1, coord_y + 1, map_width);

            const bool wall_nw = (hash::has(tiles, index_nw) && hash::get(tiles, index_nw, {}).index != floor_tile) || hash::has(placeholder_walls, index_nw);
            const bool wall_n = (hash::has(tiles, index_n) && hash::get(tiles, index_n, {}).index != floor_tile) || hash::has(placeholder_walls, index_n);
            const bool wall_ne = (hash::has(tiles, index_ne) && hash::get(tiles, index_ne, {}).index != floor_tile) || hash::has(placeholder_walls, index_ne);
            const bool wall_w = (hash::has(tiles, index_w) && hash::get(tiles, index_w, {}).index != floor_tile) || hash::has(placeholder_walls, index_w);
            const bool wall_e = (hash::has(tiles, index_e) && hash::get(tiles, index_e, {}).index != floor_tile) || hash::has(placeholder_walls, index_e);
            const bool wall_sw = (hash::has(tiles, index_sw) && hash::get(tiles, index_sw, {}).index != floor_tile) || hash::has(placeholder_walls, index_sw);
            const bool wall_s = (hash::has(tiles, index_s) && hash::get(tiles, index_s, {}).index != floor_tile) || hash::has(placeholder_walls, index_s);
            const bool wall_se = (hash::has(tiles, index_sw) && hash::get(tiles, index_se, {}).index != floor_tile) || hash::has(placeholder_walls, index_se);

            const bool floor_nw = hash::has(tiles, index_nw) && hash::get(tiles, index_nw, {}).index == floor_tile;
            const bool floor_n = hash::has(tiles, index_n) && hash::get(tiles, index_n, {}).index == floor_tile;
            const bool floor_ne = hash::has(tiles, index_ne) && hash::get(tiles, index_ne, {}).index == floor_tile;
            const bool floor_w = hash::has(tiles, index_w) && hash::get(tiles, index_w, {}).index == floor_tile;
            const bool floor_e = hash::has(tiles, index_e) && hash::get(tiles, index_e, {}).index == floor_tile;
            const bool floor_sw = hash::has(tiles, index_sw) && hash::get(tiles, index_sw, {}).index == floor_tile;
            const bool floor_s = hash::has(tiles, index_s) && hash::get(tiles, index_s, {}).index == floor_tile;
            const bool floor_se = hash::has(tiles, index_se) && hash::get(tiles, index_se, {}).index == floor_tile;

            // These signify that the walls around the coordinate are room walls, not placeholder corridor walls.
            const bool wall_w_room = !hash::has(placeholder_walls, index_w) && wall_w;
            const bool wall_e_room = !hash::has(placeholder_walls, index_e) && wall_e;
            const bool wall_n_room = !hash::has(placeholder_walls, index_n) && wall_n;
            const bool wall_s_room = !hash::has(placeholder_walls, index_s) && wall_s;

            // TODO: Handle tri-wall corners

            if (floor_e && floor_w && wall_n && wall_s) { // Vertical wall between two floor tiles
                // Do nothing
            } else if (floor_e && floor_w && floor_n && wall_s) { // Vertical wall cap north between two floor tiles
                // Do nothing
            } else if (floor_e && floor_w && floor_s && wall_n) { // Vertical wall cap south between two floor tiles
                // Do nothing
            } else if (floor_n && floor_s && wall_w && wall_e) { // Horizontal wall between two floor tiles
                // Do nothing
            } else if (floor_n && floor_s && wall_w && floor_e) { // Horizontal wall cap east between two floor tiles
                // Do nothing
            } else if (floor_n && floor_s && floor_w && wall_e) { // Horizontal wall cap west between two floor tiles
                // Do nothing
            } else if (wall_n && wall_s && floor_e) { // Vertical wall left of corridor
                hash::set(tiles, index_wall, {wall_right_tile});
            } else if (wall_n && wall_s && floor_w) { // Vertical wall right of corridor
                hash::set(tiles, index_wall, {wall_left_tile});
            } else if (wall_w && wall_e && floor_s) { // Horizontal wall, above corridor
                hash::set(tiles, index_wall, {wall_horizontal_tile});
            } else if (wall_w && wall_e && floor_n) { // Horizontal wall, below corridor
                hash::set(tiles, index_wall, {wall_horizontal_tile});
            } else if (wall_w_room && wall_n && floor_s && floor_e) { // Corner left and up, floor right and down for corridors going upward
                hash::set(tiles, index_wall, {wall_corner_bottom_right_tile});
            } else if (wall_e_room && wall_n && floor_s && floor_w) { // Corner right and up, floor left and down for corridors going upward
                hash::set(tiles, index_wall, {wall_corner_bottom_left_tile});
            } else if (wall_w_room && floor_n && wall_s && floor_e) { // Corner left and down, floor right and up for corridors going downward
                hash::set(tiles, index_wall, {wall_corner_top_right_tile});
            } else if (wall_e_room && floor_n && wall_s && floor_w) { // Corner right and down, floor left and up for corridors going downward
                hash::set(tiles, index_wall, {wall_corner_top_left_tile});
            } else if (wall_e && wall_n_room && floor_s && floor_w) { // Corner right and up, floor left and down for corridors going right
                hash::set(tiles, index_wall, {corridor_corner_up_right_tile});
            } else if (wall_e && wall_s_room && floor_n && floor_w) { // Corner right and down, floor left and up for corridors going right
                hash::set(tiles, index_wall, {corridor_corner_down_right_tile});
            } else if (wall_w && wall_n_room && floor_s && floor_e) { // Corner left and up, floor right and down for corridors going left
                hash::set(tiles, index_wall, {corridor_corner_up_left_tile});
            } else if (wall_w && wall_s_room && floor_n && floor_e) { // Corner left and down, floor right and up for corridors going left
                hash::set(tiles, index_wall, {corridor_corner_down_left_tile});
            } else if (wall_n && wall_e && floor_w && floor_s) { // Corner right and up above a corridor
                hash::set(tiles, index_wall, {wall_corner_bottom_left_tile});
            } else if (wall_n && wall_e && floor_ne) { // Corner right and up below a corridor
                hash::set(tiles, index_wall, {corridor_corner_up_right_tile});
            } else if (wall_w && wall_s && floor_n && floor_e) { // Corner left and down below a corridor
                hash::set(tiles, index_wall, {wall_corner_top_right_tile});
            } else if (wall_w && wall_s && floor_sw) { // Corner left and down above a corridor
                hash::set(tiles, index_wall, {corridor_corner_down_left_tile});
            } else if (wall_e && wall_s && floor_se) { // Corner right and down above a corridor
                hash::set(tiles, index_wall, {corridor_corner_down_right_tile});
            } else if (wall_e && wall_s && floor_nw) { // Corner right and down below a corridor
                hash::set(tiles, index_wall, {wall_corner_top_left_tile});
            } else if (wall_w && wall_n && floor_se) { // Corner left and up above a corridor
                hash::set(tiles, index_wall, {wall_corner_bottom_right_tile});
            } else if (wall_w && wall_n && floor_nw) { // Corner left and up below a corridor
                hash::set(tiles, index_wall, {corridor_corner_up_left_tile});
            } else {
                hash::set(tiles, index_wall, {debug_tile});
            }
        };

        // Update placeholder walls
        for (auto iter = hash::begin(placeholder_walls); iter != hash::end(placeholder_walls); ++iter) {
            int32_t index = (int32_t)iter->key;
            place_wall(index);
        }
    }

    // Update game's state.
    if (SDL_LockMutex(game->mutex) == 0) {
        game->terrain_tiles = tiles;
        game->max_width = map_width;
        SDL_UnlockMutex(game->mutex);
    } else {
        log_fatal("Could not lock mutex %s", SDL_GetError());
    }

    log_info("DunGen completed");

    transition(*game, GameState::Playing);

    return 0;
}

} // namespace game
