#pragma once
#include <RealEngine/main/MainProgram.hpp>

#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>


class RealWorld : public RE::MainProgram {
public:
	RealWorld(RE::CommandLineArguments args);
	~RealWorld();
private:
	struct Databases {
		Databases();
	};
	Databases m_databases;		/**< Fake object that forces datatabase initialization */
	WorldRoom m_worldRoom;		/**< Huge objects but it does not matter since this will be in static memory */
	MainMenuRoom m_mainMenuRoom;/**< Huge objects but it does not matter since this will be in static memory */
};