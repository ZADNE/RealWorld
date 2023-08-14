/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/program/MainProgram.hpp>

#include <RealWorld/main/MainMenuRoom.hpp>
#include <RealWorld/main/WorldRoom.hpp>
#include <RealWorld/main/settings/GameSettings.hpp>

int main(int argc, char* argv[]) {
    re::MainProgram::initialize();

    rw::GameSettings gameSettings{};

    auto* mainMenuRoom = re::MainProgram::addRoom<rw::MainMenuRoom>(gameSettings);
    re::MainProgram::addRoom<rw::WorldRoom>(gameSettings);

    return re::MainProgram::run(mainMenuRoom->name(), {});
}