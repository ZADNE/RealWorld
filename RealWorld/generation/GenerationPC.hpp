/*!
 *  @author    Dubsky Tomas
 */
#pragma once

#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Represents push constants used during generation
 */
struct GenerationPC {
    glm::ivec2 chunkOffsetTi;
    int        seed;
    glm::uint  storeLayer;
    glm::uint  edgeConsolidationPromote;
    glm::uint  edgeConsolidationReduce;
};

} // namespace rw