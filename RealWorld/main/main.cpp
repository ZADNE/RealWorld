/*! 
 *  @author    Dubsky Tomas
 */
#include <RealEngine/program/MainProgram.hpp>

#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>

int main(int argc, char* argv[]) {
    RE::MainProgram::initialize();

    GameSettings gameSettings{};

    auto* mainMenuRoom = RE::MainProgram::addRoom<MainMenuRoom>(gameSettings);
    RE::MainProgram::addRoom<WorldRoom>(gameSettings);

    return RE::MainProgram::run(mainMenuRoom->getName(), {});
}