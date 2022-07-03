/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/RealWorld.hpp>

RealWorld::RealWorld(RE::CommandLineArguments args) :
	m_worldRoom(m_gameSettings),
	m_mainMenuRoom(m_gameSettings) {

	p_window.setTitle("RealWorld!");

	auto mainRoom = p_roomManager.addRoom(&m_mainMenuRoom);
	p_roomManager.addRoom(&m_worldRoom);

	p_roomManager.gotoRoom(mainRoom, {});
}

RealWorld::~RealWorld() {

}
