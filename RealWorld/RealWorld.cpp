#include <RealWorld/RealWorld.hpp>

#include <RealWorld/world/WorldRoom.hpp>
#include <RealWorld/menu/MainMenuRoom.hpp>

RealWorld::RealWorld(RE::CommandLineArguments args) : RE::MainProgram(args) {
	p_window.setTitle("Realworld");

	p_roomManager.addRoom<MainMenuRoom>(args);
	p_roomManager.addRoom<WorldRoom>(args);

	p_roomManager.gotoRoom(0);
}

RealWorld::~RealWorld() {

}