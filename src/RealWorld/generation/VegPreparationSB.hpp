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
    glm::uint outputIndex;   // Index to the branch buffers (vector and raster)
    glm::uint randomSeed;
    glm::uint branchPrepInstIndex; // Index into b_vegPrep.branchInstances
    glm::vec2 rootPosTi;
    float sizeFactor;
    float angleFactor;
};

struct BranchInstance {
    glm::vec2 offsetTi;
    glm::uvec2 sizeTi;
    glm::uint wallType;
    glm::uint outputIndex; // Index to the branch buffers (vector and raster)
    glm::uint randomSeed;
};

/**
 * @brief Is C++ representation with the same layout as the GLSL storage buffer
 */
struct VegPreparationSB {
    glm::uvec4 vegDispatchSize;
    glm::uvec4 branchDispatchSize;
    VegInstance vegInstances[k_chunkGenSlots * 64];
    BranchInstance branchInstances[k_chunkGenSlots * 64 * 64];
};

} // namespace rw
