#include <RealWorld/main/RealWorld.hpp>

#include <RealWorld/performance_tests/switches.hpp>

#ifdef MEASURE_GENERATION_DELAY
#include <RealWorld/save/WorldSaveLoader.hpp>
#endif // MEASURE_GENERATION_DELAY

RealWorld::RealWorld(RE::CommandLineArguments args) :
	m_worldRoom(args),
	m_mainMenuRoom(args) {

	p_window.setTitle("RealWorld!");

#ifndef MEASURE_GENERATION_DELAY
	auto mainRoom = p_roomManager.addRoom(&m_mainMenuRoom);
	p_roomManager.addRoom(&m_worldRoom);

	p_roomManager.gotoRoom(mainRoom, {});
#else
	auto worldRoom = p_roomManager.addRoom(&m_worldRoom);
	auto seed = static_cast<int>(time(nullptr)) & 65535;
	auto name = "PERF_TEST_" + std::to_string(seed);
	p_window.setVSync(false, false);
	resizeWindow(glm::ivec2(1920, 1080), false);
	if (WorldSaveLoader::createWorld(name, seed)) {
		p_roomManager.gotoRoom(worldRoom, {name});
	}
#endif // MEASURE_GENERATION_DELAY
}

RealWorld::~RealWorld() {

}
