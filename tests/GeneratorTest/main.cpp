/*! 
 *  @author    Dubsky Tomas
 */
#include <chrono>
#include <optional>

#include <RealEngine/main/room/Room.hpp>
#include <RealEngine/graphics/View.hpp>

#include <RealWorld/chunk/ChunkGeneratorCS.hpp>
#include <RealWorld/chunk/ChunkGeneratorFBO.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/world/WorldDrawer.hpp>

const glm::ivec2 RESOLUTION = glm::ivec2(1920, 1080);
const unsigned int FPS_LIMIT = 300u;
const glm::vec4 SKY_BLUE = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

using namespace std::chrono;

class Room : public RE::Room {
public:
	Room(RE::CommandLineArguments args) : m_worldDrawer(RESOLUTION) {}
	~Room() {}

	virtual void sessionStart(const RE::RoomTransitionParameters& params) override {
		resetRecords();
		//Initialize compute shader generator
		m_genCS.emplace();
		m_world.emplace(*m_genCS);
		WorldSave save{.metadata = MetadataSave{ .seed = static_cast<int>(time(nullptr)) & 65535}};
		auto worldTextureSize = (*m_world).adoptSave(save.metadata, RESOLUTION);
		m_worldDrawer.setTarget(worldTextureSize);
		std::cout << "Check console once the view stops moving (~1.5 min).\n";
	}

	virtual void sessionEnd() override {}

	virtual void step() override {
		m_worldView.setPosition(m_worldView.getPosition() + glm::vec2(10.0f, 0.0f));
		auto viewEnvelope = m_worldDrawer.setPosition(m_worldView.getBotLeft());

		auto start = steady_clock::now();
		int activatedChunks = (*m_world).step(viewEnvelope.botLeftTi, viewEnvelope.topRightTi);
		if (activatedChunks > 0) {
			m_batchN++;
			if (m_batchN > 0) {
				glFinish();
				auto dur = steady_clock::now() - start;
				m_total_ns += dur;
				int N = duration_cast<microseconds>(dur).count() / 1000;
				m_histogram[glm::clamp(N, 0, 9)]++;
				if (m_batchN % 100 == 0) {
					std::cout << (m_state == 0 ? "CS" : "VS+FS+FBO") << ": 100 batches histogram \\|/\n";
					for (size_t i = 0; i < 10; ++i) {
						std::cout << i << "ms\t";
					}
					std::cout << "\n";
					for (size_t i = 0; i < 10; ++i) {
						std::cout << "    " << m_histogram[i] << "\t";
					}
					std::cout << "\nAverage time: " << duration_cast<microseconds>(m_total_ns / m_batchN).count() << "\n";
					m_state++;
					if (m_state == 1){
						//Switch to framebuffer generator
						resetRecords();
						m_world.reset();
						m_genCS.reset();
						m_genFBO.emplace();
						m_world.emplace(*m_genFBO);
						WorldSave save{.metadata = MetadataSave{ .seed = static_cast<int>(time(nullptr)) & 65535}};
						auto worldTextureSize = (*m_world).adoptSave(save.metadata, RESOLUTION);
						m_worldDrawer.setTarget(worldTextureSize);
					} else {
						std::string dontcare;
						std::cin >> dontcare;
						program()->scheduleProgramExit(0);
					}
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

	void resetRecords(){
		m_batchN = -5;
		m_total_ns = nanoseconds::zero();
		for (int i = 0; i < 10; i++){
			m_histogram[i] = 0;
		}
	}

private:
	RE::View m_worldView{RESOLUTION};
	std::optional<ChunkGeneratorCS> m_genCS;
	std::optional<ChunkGeneratorFBO> m_genFBO;
	std::optional<World> m_world;
	WorldDrawer m_worldDrawer{RESOLUTION};
	int m_batchN;
	nanoseconds m_total_ns;
	int m_histogram[10];
	int m_state;
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