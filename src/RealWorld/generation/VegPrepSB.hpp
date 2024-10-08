/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <RealWorld/constants/generation.hpp>

namespace rw {

// NOLINTBEGIN: Shader mirror
struct alignas(sizeof(glm::vec2)) VegInstance {
    glm::vec2 rootPosTi;
    glm::uint templateIndex;
    glm::uint chunkIndex;
    glm::uint randomSeed;
    float growth;
    float rootDirNorm;
    float tropismDirNorm;
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
// NOLINTEND

} // namespace rw
