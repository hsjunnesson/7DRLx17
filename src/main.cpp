#include <SDL.h>
#include <SDL_image.h>

#include "window.hpp"
#include "log.hpp"
#include "texture.hpp" 
#include "game_loop.hpp"

#include <exception>
#include <iostream>


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480


static void init_systems(void) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		quit_fmt("SDL_Init: %s", SDL_GetError());
	}

	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		quit_fmt("IMG_Init: %s", IMG_GetError());
	}
}

Window *init_window() {
	SDL_Rect rect;
	rect.x = 100;
	rect.y = 100;
	rect.w = 640;
	rect.h = 480;

	SDL_Window *sdl_window = SDL_CreateWindow(
		"Roguelike",
		rect.x, rect.y,
		rect.w, rect.h,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (sdl_window == NULL) {
		quit_fmt("SDL_CreateWindow: %s", SDL_GetError());
	}

	SDL_Renderer *sdl_renderer = SDL_CreateRenderer(
		sdl_window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (sdl_renderer == NULL) {
		SDL_DestroyWindow(sdl_window);
		quit_fmt("SDL_CreateRenderer: %s", SDL_GetError());
	}

	return new Window(rect, *sdl_window, *sdl_renderer);
}

static void run() {
	init_systems();
	Window *window = init_window();
	World *world = new World();
	InputManager *input_manager = new InputManager();

	GameLoop *game_loop = new GameLoop(*window, *world, *input_manager);
	game_loop->run();

	delete window;
	
	SDL_Quit();
}

using namespace std;

int main(int argc, char* argv[]) {
	try {
		run();
	}
	catch (const std::exception & e) {
		cout << e.what() << endl;
		return 1;
	}

	return 0;
}
