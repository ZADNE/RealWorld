/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/main/RealWorld.hpp>

RealWorld::RealWorld(RE::CommandLineArguments args) :
	m_worldRoom(m_gameSettings),
	m_mainMenuRoom(m_gameSettings) {

	m_window.setTitle("RealWorld!");

	auto mainRoom = m_roomManager.addRoom(&m_mainMenuRoom);
	m_roomManager.addRoom(&m_worldRoom);

	m_roomManager.gotoRoom(mainRoom, {});
}
