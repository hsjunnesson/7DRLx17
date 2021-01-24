#pragma once

#include <SDL.h>


class World {

public:

  World();

  /**
   * @brief Updates the world
   *
   * @param t The current time
   * @param dt The delta time since last update
   */
  void update(Uint32 t, double dt);

private:

};

