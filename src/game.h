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

// The namespace for all of the game specific gameplay code.
namespace game {
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
static uint64_t CorridorCornerUpRight = hash("corridor_corner_up_right");
static uint64_t CorridorCornerUpLeft = hash("corridor_corner_up_left");
static uint64_t CorridorCornerDownRight = hash("corridor_corner_down_right");
static uint64_t CorridorCornerDownLeft = hash("corridor_corner_down_left");

static uint64_t StairsUp = hash("stairs_up");
static uint64_t StairsDown = hash("stairs_down");

static uint64_t Floor = hash("floor");

static uint64_t Player = hash("player");
static uint64_t Ghost = hash("ghost");
static uint64_t Snake = hash("snake");

static uint64_t Missing = hash("missing");

} // namespace tile

/**
 * @brief A tile struct.
 * 
 */
struct Tile {
    int32_t index = -1;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    double angle = 0.0;
};

/**
 * @brief A room
 * 
 */
struct Room {
    int32_t room_index;
    int32_t x, y;
    int32_t w, h;
    bool start_room = false;
    bool boss_room = false;
};

/**
 * @brief A corridor between rooms
 * 
 */
struct Corridor {
    int32_t from_room_index;
    int32_t to_room_index;
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

/**
 * @brief A dungeon level.
 * 
 */
struct Level {
    int32_t stairs_down_pos;
    int32_t stairs_up_pos;
    int32_t depth;
};

// The game state.
struct Game {
    Game(Allocator &allocator, SDL_Renderer *renderer, const char *atlas_config_filename);
    ~Game();

    // The allocator
    Allocator &allocator;

    // The current game state.
    GameState game_state;

    // The Game mutex;
    SDL_mutex *mutex;

    // The thread for generating a dungeon.
    SDL_Thread *dungen_thread;

    // The texture atlas
    texture::Atlas atlas;

    // The depth level of the current game.
    Level level;

    // The camera x and y offset
    int x_offset, y_offset;

    // The camera zoom
    int zoom_level;

    // The hash of terrain tile states.
    Hash<Tile> terrain_tiles;

    // The hash of entity tile states.
    Hash<Tile> entity_tiles;

    // The maximum width in tiles. Needed for index to coord translation
    uint32_t max_width;

    // The player's position as an index.
    int32_t player_pos;
};

/**
 * @brief Updates the game
 *
 * @param game The game to update
 * @param t The current time
 * @param dt The delta time since last update
 */
void update(Game &game, uint32_t t, double dt);

/**
 * @brief Callback to the game that an input has ocurred.
 * 
 * @param game The game to signal.
 * @param input_command The input command.
 */
void on_input(Game &game, input::InputCommand input_command);

/**
 * @brief Renders the game
 *
 * @param game The game to render
 * @param renderer The SDL renderer.
 */
void render(Game &game, SDL_Renderer *renderer);

/**
 * @brief Transition a Game from one game state to another
 * 
 * @param game The game to transition
 * @param game_state The GameState to transition to.
 */
void transition(Game &game, GameState game_state);

/**
 * @brief Returns the index of an x, y coordinate
 * 
 * @param x The x coord
 * @param y The y coord
 * @param max_width The maximum width.
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
 * @param max_width The maximum width.
 */
constexpr void coord(int32_t const index, int32_t &x, int32_t &y, int32_t const max_width) {
    x = index % max_width;
    y = index / max_width;
}

/**
 * @brief Returns a new index offset by x and y coordinates.
 * 
 * @param idx The index.
 * @param xoffset The x offset.
 * @param yoffset They y offset.
 * @param max_width The maximum width.
 * @return constexpr int32_t The index.
 */
constexpr int32_t index_offset(int32_t const idx, int32_t const xoffset, int32_t const yoffset, int32_t const max_width) {
    int32_t x = 0;
    int32_t y = 0;
    coord(idx, x, y, max_width);

    return index(x + xoffset, y + yoffset, max_width);
}

} // namespace game
