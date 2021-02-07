#include "dungen.h"

#include "proto/engine.pb.h"

#include "world.h"
#include "log.h"
#include "config.h"

namespace world {
    int dungen_thread(void *data) {
        World *world = (World *)data;
        if (!world) {
            log_fatal("No World in dungen_thread");
        }

        engine::DunGenParams params;
        config::read("assets/dungen_params.json", &params);

        if (SDL_LockMutex(world->mutex) == 0) {
            for (int y = 0; y < params.map_height(); ++y) {
                for (int x = 0; x < params.map_width(); ++x) {
                    hash::set(world->tiles, index(x, y, params.map_width()), {rand() % 140});
                }
            }

            world->max_width = params.map_width();
            
            SDL_UnlockMutex(world->mutex);
        } else {
            log_fatal("Could not lock mutex %s", SDL_GetError());
        }

        log_info("DunGen completed");

        transition(*world, GameState::Playing);

        return 0;
    }
}
