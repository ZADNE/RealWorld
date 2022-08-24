/*! 
 *  @author    Dubsky Tomas
 */
#include <RealEngine/main/program/MainProgram.hpp>

#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>

int main(int argc, char* argv[]) {
	RE::MainProgram::initialize();

	GameSettings gameSettings;					/**< Loads and stores settings specific to the RealWorld game */
	WorldRoom worldRoom{gameSettings};			/**< Room with the gameplay */
	MainMenuRoom mainMenuRoom{gameSettings};	/**< Room with the main menu */

	return RE::MainProgram::run(mainMenuRoom, {});
}