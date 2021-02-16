#include "dungen.h"

#include <random>

#pragma warning(push, 0)
#include "proto/engine.pb.h"
#include "memory.h"
#include "hash.h"
#include "array.h"
#pragma warning(pop)

#include "world.h"
#include "log.h"
#include "config.h"

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

        const int64_t rooms_width = (int64_t)floor(sqrt(params.room_count()));
        const int64_t rooms_height = (int64_t)ceil(params.room_count() / rooms_width);
        const int64_t section_width = params.map_width() / rooms_width;
        const int64_t section_height = params.map_height() / rooms_height;

        std::random_device random_device;
        std::mt19937 random_engine(random_device());
        std::uniform_int_distribution<int64_t> room_size_distribution(params.min_room_size(), params.max_room_size());

        Array<RoomRect> rooms = Array<RoomRect>(allocator);

        for (int room_index = 0; room_index < params.room_count(); ++room_index) {
            uint64_t room_index_x, room_index_y;
            coord(room_index, room_index_x, room_index_y, rooms_width);

            const uint64_t section_min_x = room_index_x * section_width;
            const uint64_t section_max_x = section_min_x + section_width;
            const uint64_t section_min_y = room_index_y * section_height;
            const uint64_t section_max_y = section_min_y + section_height;

            const uint64_t room_width = room_size_distribution(random_engine);
            const uint64_t room_height = room_size_distribution(random_engine);

            const uint64_t room_x = section_min_x + section_width / 2 - room_width / 2;
            const uint64_t room_y = section_min_y + section_height / 2 - room_height / 2;

            array::push_back(rooms, RoomRect {room_x, room_y, room_width, room_height});
        }

        for (auto iter = array::begin(rooms); iter != array::end(rooms); ++iter) {
            RoomRect room = *iter;

            for (int y = 0; y < room.h; ++y) {
                for (int x = 0; x < room.w; ++x) {
                    int32_t tile_index = hash::get(world->atlas->tiles_by_name, tile::Snake, 0);
                    hash::set(tiles, index(room.x + x, room.y + y, map_width), {tile_index});
                }
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
}
