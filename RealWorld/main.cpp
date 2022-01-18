#include <SDL2/SDL_main.h>

#include <RealWorld/Game.hpp>

int main(int argc, char* argv[]) {
	Game game;
	game.runProgram(argc, argv);
	return EXIT_SUCCESS;
}