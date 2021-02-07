#include "dungen.h"

#include "world.h"
#include "log.h"

namespace world {
    int dungen_thread(void *data) {
        World *world = (World *)data;
        if (!world) {
            log_fatal("No World in dungen_thread");
        }

        if (SDL_LockMutex(world->mutex) == 0) {
            for (int y = 0; y < Max_Height; ++y) {
                for (int x = 0; x < Max_Width; ++x) {
                    hash::set(world->tiles, index(x, y, Max_Width), {rand() % 140});
                }
            }

            SDL_UnlockMutex(world->mutex);
        } else {
            log_fatal("Could not lock mutex %s", SDL_GetError());
        }

        log_info("DunGen completed");

        transition(*world, GameState::Playing);

        return 0;
    }
}
