/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/main/MainProgram.hpp>

#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>

/**
 * @brief Is a container for everything of the RealWorld game.
 * 
 * This is the main program-class with a room for main menu and gameplay.
*/
class RealWorld : public RE::MainProgram {
public:
	RealWorld(RE::CommandLineArguments args);
	~RealWorld();
private:
	WorldRoom m_worldRoom;		/**< Room with the gameplay */
	MainMenuRoom m_mainMenuRoom;/**< Room with the main menu */
};