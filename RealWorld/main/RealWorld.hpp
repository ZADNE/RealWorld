/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/main/program/MainProgram.hpp>

#include <RealWorld/main/settings/GameSettings.hpp>
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

private:
	GameSettings m_gameSettings;/**< Loads and stores settings specific to the RealWorld game */
	WorldRoom m_worldRoom;		/**< Room with the gameplay */
	MainMenuRoom m_mainMenuRoom;/**< Room with the main menu */
};