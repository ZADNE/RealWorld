/*! 
 *  @author    Dubsky Tomas
 */
#include <chrono>

#include <RealEngine/main/room/Room.hpp>
#include <RealEngine/graphics/View.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/WorldDrawer.hpp>

const glm::ivec2 RESOLUTION = glm::ivec2(1920, 1080);
const unsigned int FPS_LIMIT = 300u;
const glm::vec4 SKY_BLUE = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

using namespace std::chrono;

class Room : public RE::Room {
public:
	Room(RE::CommandLineArguments args) :
		m_world(),
		m_worldDrawer(RESOLUTION) {

	}
	~Room() {}

	virtual void sessionStart(const RE::RoomTransitionParameters& params) override {
		WorldSave save{.metadata = MetadataSave{
			.path = "",
			.seed = static_cast<int>(time(nullptr)) & 65535,
			.worldName = "PERF_TEST"
		}};
		auto worldTextureSize = m_world.adoptSave(save.metadata, window()->getDims());
		m_worldDrawer.setTarget(worldTextureSize);
		std::cout << "Check the console once the view stops.\n";
	}

	virtual void sessionEnd() override {}

	virtual void step() override {
		m_worldView.setPosition(m_worldView.getPosition() + glm::vec2(10.0f, 0.0f));
		auto viewEnvelope = m_worldDrawer.setPosition(m_worldView.getBotLeft());

		auto start = steady_clock::now();
		int activatedChunks = m_world.step(viewEnvelope.botLeftTi, viewEnvelope.topRightTi);
		if (activatedChunks > 0) {
			batchN++;
			if (batchN > 0) {
				glFinish();
				auto dur = steady_clock::now() - start;
				total_ns += dur;
				auto N = duration_cast<microseconds>(dur).count() / 1000 - 3;
				histogram[glm::clamp(N, 0ll, 9ll)]++;
				if (batchN % 100 == 0) {
					std::cout << "100 batches generated! Histogram \\|/\n";
					for (size_t i = 3; i < 14; ++i) {
						std::cout << i << "ms\t";
					}
					std::cout << "\n";
					for (size_t i = 0; i < 10; ++i) {
						std::cout << "    " << histogram[i] << "\t";
					}
					std::cout << "\nAverage time: " << duration_cast<microseconds>(total_ns / batchN) << "\n";
					std::string dontcare;
					std::cin >> dontcare;
					program()->scheduleProgramExit(0);
				}
			}
		}

		m_worldDrawer.beginStep();
		m_worldDrawer.addLight(m_worldView.getPosition(), RE::Color{0u, 0u, 0u, 100u});
		m_worldDrawer.endStep();
	}

	virtual void render(double interpolationFactor) override {
		m_worldDrawer.drawTiles();
		m_worldDrawer.coverWithShadows();
	}

	virtual const RE::RoomDisplaySettings& getDisplaySettings() override {
		static RE::RoomDisplaySettings settings{
			.clearColor = SKY_BLUE,
			.stepsPerSecond = PHYSICS_STEPS_PER_SECOND,
			.framesPerSecondLimit = FPS_LIMIT,
			.usingImGui = true
		};
		return settings;
	}
private:
	RE::View m_worldView{RESOLUTION};
	World m_world;
	WorldDrawer m_worldDrawer;
	int batchN = -5;
	nanoseconds total_ns = nanoseconds::zero();
	int histogram[10] = {0};
};

class Program : public RE::MainProgram {
public:
	Program(RE::CommandLineArguments args) :
		m_testRoom(args) {
		resizeWindow(RESOLUTION, false);
		p_roomManager.gotoRoom(p_roomManager.addRoom(&m_testRoom), {});
	}
	~Program() {}
private:
	Room m_testRoom;
};

int main(int argc, char* argv[]) {
	return RE::runProgram<Program>(argc, argv);
}