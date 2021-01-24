#include <SDL.h>
#include "log.hpp"

#include <stdio.h>


#define EXIT_FAILURE 1

void logSDLError(const char* message) {
	fprintf(stderr, "%s: %s\n", message, SDL_GetError());
}

void quit_fmt(const char* fmt, ...) {
	va_list(args);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n");

	SDL_Quit();
	exit(EXIT_FAILURE);
}
