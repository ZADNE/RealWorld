/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace rw {

struct VegInstance {
    glm::uint templateRootIndex; // Index to the template buffer
    glm::uint writeIndex;        // Index to the branch buffer
    glm::uint randomSeed;
    glm::uint branchBaseIndex;
    glm::vec2 rootPosTi;
    float     sizeFactor;
    float     angleFactor;
};

struct BranchInstance {
    glm::vec2  offsetTi;
    glm::uvec2 sizeTi;
    glm::uint  wallType;
    glm::uint  writeIndex;  // Index to the branch buffers (vector and raster)
    glm::uint  randomSeed;
    glm::uint  padding;
};

/**
 * @brief Is C++ representation with the same layout as the GLSL storage buffer
 */
struct VegPreparationSB {
    glm::uvec4     b_vegetationDispatchSize;
    glm::uvec4     b_branchDispatchSize;
    VegInstance    b_vegInstances[32];
    BranchInstance b_branchInstances[32 * 64];
};

} // namespace rw
