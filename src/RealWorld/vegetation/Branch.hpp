/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace rw {

/**
 * @brief   Is a solid section of a vegetation
 * @details A branch is always attached to end of a parent branch.
 *          The root branch is parent of itself.
 *          Vegetation may only bend in pivots that connect these sections.
 * @see     VegSimulator
 */
struct Branch {
    glm::vec2 absPosTi; // Position of the end that is connected to parent
    int parentIndexOffset;
    glm::uint wallType;
    float absAngleNorm;     // Current absolute angle of the branch
    float relRestAngleNorm; // Branch tries to stay in this relative angle to parent
    float angleVelNorm;
    float radiusTi;
    float lengthTi;
    float density;
    float stiffness; // Resistance to bending
    glm::uint padding;
};

} // namespace rw
