#pragma once

#include "input.h"
#include "texture.h"

#pragma warning(push, 0)
#include "array.h"
#include "hash.h"
#include "memory.h"
#include "murmur_hash.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#pragma warning(pop)

// The namespace for all of the game world specific gameplay code.
namespace world {
using namespace foundation;

// A namespace of named, known tiles.
namespace tile {

// Murmur hashes a string.
uint64_t hash(const char *s);

static uint64_t WallCornerTopLeft = hash("wall_corner_top_left");
static uint64_t WallHorizontal = hash("wall_horizontal");
static uint64_t WallCornerTopRight = hash("wall_corner_top_right");
static uint64_t WallLeft = hash("wall_left");
static uint64_t WallRight = hash("wall_right");
static uint64_t WallCornerBottomLeft = hash("wall_corner_bottom_left");
static uint64_t WallCornerBottomRight = hash("wall_corner_bottom_right");
static uint64_t Floor = hash("floor");

static uint64_t Ghost = hash("ghost");
static uint64_t Snake = hash("snake");

static uint64_t Missing = hash("missing");

} // namespace tile

/**
 * @brief A tile struct.
 * 
 */
struct Tile {
    int32_t index = 0;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    double angle = 0.0;
};

/**
 * @brief An enum that describes a specific game state.
 * 
 */
enum class GameState {
    // No game state.
    None,

    // Game state is creating, or loading from a save.
    Initializing,

    // Generating a dungeon level.
    DunGen,

    // Playing the game.
    Playing,

    // Shutting down, saving and closing the game.
    Quitting,

    // Final state that signals the engine to terminate the application.
    Terminate,
};

// The game world.
struct World {
    World(Allocator &allocator, SDL_Renderer *renderer, const char *atlas_config_filename);
    ~World();

    // The allocator
    Allocator &allocator;

    // The current game state.
    GameState game_state;

    // The World mutex;
    SDL_mutex *mutex;

    // The thread for generating a dungeon.
    SDL_Thread *dungen_thread;

    // The texture atlas
    texture::Atlas atlas;

    // The camera x and y offset
    int x_offset, y_offset;

    // The The hash of tile states.
    Hash<Tile> tiles;

    // The maximum width in tiles. Needed for index to coord translation
    uint32_t max_width;
};

/**
 * @brief Updates the world
 *
 * @param world The world to update
 * @param t The current time
 * @param dt The delta time since last update
 */
void update(World &world, uint32_t t, double dt);

/**
 * @brief Callback to the world that an input has ocurred.
 * 
 * @param world The world to signal.
 * @param input_command The input command.
 */
void on_input(World &world, input::InputCommand input_command);

/**
 * @brief Renders the world
 *
 * @param world The world to render
 * @param renderer The SDL renderer.
 */
void render(World &world, SDL_Renderer *renderer);

/**
 * @brief Transition a World from one game state to another
 * 
 * @param world The world to transition
 * @param game_state The GameState to transition to.
 */
void transition(World &world, GameState game_state);

/**
 * @brief Returns the index of an x, y coordinate
 * 
 * @param x The x coord
 * @param y The y coord
 * @param max_width The maxium width.
 * @return constexpr int32_t The index.
 */
constexpr int32_t index(int32_t const x, int32_t const y, int32_t const max_width) {
    return x + max_width * y;
}

/**
 * @brief Calculates the x, y coordinates based on an index.
 * 
 * @param index The index.
 * @param x The pass-by-reference x coord to calculate.
 * @param y The pass-by-reference y coord to calculate.
 * @param max_width The maxium width.
 */
constexpr void coord(int32_t const index, int32_t &x, int32_t &y, int32_t const max_width) {
    x = index % max_width;
    y = index / max_width;
}

} // namespace world
