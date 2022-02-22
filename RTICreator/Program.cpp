#include "Program.hpp"


Program::Program(RE::CommandLineArguments args) :
	m_mainMenuRoom(args) {
	p_window.setTitle("rti Creator v2.0.0");

	auto mainRoom = p_roomManager.addRoom(&m_mainMenuRoom);

	p_roomManager.gotoRoom(mainRoom, {});
}

Program::~Program() {

}
