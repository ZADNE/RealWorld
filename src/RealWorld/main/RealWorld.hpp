#pragma once
#include <RealEngine/main/MainProgram.hpp>

#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>

/**
 * @brief Is the main program-class of RealWorld.
*/
class RealWorld : public RE::MainProgram {
public:
	RealWorld(RE::CommandLineArguments args);
	~RealWorld();
private:
	struct Database {
		Database();
	};
	Database m_database;		/**< Empty object that forces datatabase initialization */
	WorldRoom m_worldRoom;		/**< Huge objects but it does not matter since this will be in static memory */
	MainMenuRoom m_mainMenuRoom;/**< Huge objects but it does not matter since this will be in static memory */
};