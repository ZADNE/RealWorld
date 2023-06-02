/*! 
 *  @author    Dubsky Tomas
 */
#include <RealEngine/program/MainProgram.hpp>

#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>

int main(int argc, char* argv[]) {
    re::MainProgram::initialize();

    GameSettings gameSettings{};

    auto* mainMenuRoom = re::MainProgram::addRoom<MainMenuRoom>(gameSettings);
    re::MainProgram::addRoom<WorldRoom>(gameSettings);

    return re::MainProgram::run(mainMenuRoom->name(), {});
}