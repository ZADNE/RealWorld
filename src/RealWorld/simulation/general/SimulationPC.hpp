/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Union of push constants used for simulation
 */
union SimulationPC {
    struct WorldDynamics {
        glm::ivec2 playerPosTi;
        glm::ivec2 globalOffsetTi;
        glm::ivec2 worldTexMaskTi;
        glm::uint modifyLayer;
        glm::uint modifyShape;
        glm::uvec2 modifySetValue;
        int modifyMaxCount;
        float modifyRadius;
        glm::uint timeHash;
        glm::uint updateOrder = 0b00011011'00011011'00011011'00011011;
        float timeSec         = static_cast<float>(time(nullptr) & 0xFFFF);
    } worldDynamics;

    struct DroppedTilesSim {
        glm::vec2 playerBotLeftPx;
        glm::vec2 playerDimsPx;
        glm::ivec2 worldTexMaskTi;
        float timeSec;
    } droppedTilesSim;
};

} // namespace rw
