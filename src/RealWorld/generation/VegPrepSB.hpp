/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <RealWorld/constants/generation.hpp>

namespace rw {

struct VegInstance {
    glm::uint templateIndex; // Index of the template
    glm::uint randomSeed;
    glm::uint chunkIndex;
    float growth;
    glm::vec2 rootPosTi;
};

struct VegPrepSB {
    glm::uvec4 vegDispatchSize;
    glm::uvec4 branchDispatchSize;
    VegInstance vegInstances[k_chunkGenSlots * k_maxVegPerChunk];
    glm::uint vegOffsetWithinChunk[k_chunkGenSlots * k_maxVegPerChunk];
    glm::uint prepIndexOfFirstBranch[k_chunkGenSlots * k_maxVegPerChunk];
    glm::uint branchOfChunk[k_chunkGenSlots];

    // Branches
    glm::uint vegIndex[k_branchGenSlots];
    glm::vec2 absPosTi[k_branchGenSlots];
    float absAngNorm[k_branchGenSlots];
    glm::uint parentOffset15wallType31[k_branchGenSlots]; // explanation below
    float relRestAngNorm[k_branchGenSlots]; // rest angle relative to parent
    float radiusTi[k_branchGenSlots];
    float lengthTi[k_branchGenSlots];
    glm::vec2 densityStiffness[k_branchGenSlots];
};

} // namespace rw
