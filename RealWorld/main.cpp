#include <SDL\SDL_main.h>

#include "Game.hpp"

int main(int argc, char* argv[]) {
	Game game;
	game.runProgram(argc, argv);
	return EXIT_SUCCESS;
}