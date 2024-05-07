/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <print>

#include <RealWorld/main/WorldRoom.hpp>

namespace rw::perf_test {

/**
 * @brief Measures average FPS over 30 seconds of simulation
 */
class TestRoom: public rw::WorldRoom {
public:
    explicit TestRoom(const rw::GameSettings& gameSettings)
        : rw::WorldRoom{gameSettings} {}

    void sessionStart(const re::RoomTransitionArguments& args) override {
        engine().setWindowDims(glm::ivec2{1920, 1080}, false);
        rw::WorldRoom::sessionStart(args);
    }

    void step() override {
        m_stepsSimulated++;
        if (m_stepsSimulated >= 30 * rw::k_physicsStepsPerSecond) {
            std::println(
                "Perf test finished - average FPS: {0}", m_framesRendered / 30.0
            );
            std::exit(0); // Hard exit to skip saving of chunks
        }
        rw::WorldRoom::step();
    }

    void render(const re::CommandBuffer& cb, double interpolationFactor) override {
        m_framesRendered++;
        rw::WorldRoom::render(cb, interpolationFactor);
    }

private:

    int m_stepsSimulated = 0;
    int m_framesRendered = 0;
};

} // namespace rw::perf_test
