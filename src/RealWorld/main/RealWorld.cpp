#include <RealWorld/main/RealWorld.hpp>

#include <RealWorld/world/TDB.hpp>

RealWorld::RealWorld(RE::CommandLineArguments args) :
	m_worldRoom(args),
	m_mainMenuRoom(args) {

	p_window.setTitle("Realworld");


	auto mainRoom = p_roomManager.addRoom(&m_mainMenuRoom);
	p_roomManager.addRoom(&m_worldRoom);

	p_roomManager.gotoRoom(mainRoom, {});
}

RealWorld::~RealWorld() {

}

RealWorld::Databases::Databases() {
	IDB::init();
	TDB::init();
}
