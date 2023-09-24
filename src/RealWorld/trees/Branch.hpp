﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace rw {

/**
 * @brief   Is a solid section of a tree
 * @details A branch is always attached to end of a parent branch.
 *          The root branch is parent of itself.
 *          Trees may only bend in pivots that connect these sections.
 * @see     TreeSimulator
 */
struct Branch {
    glm::vec2 absPosTi; // Position of the end that is connected to parent
    glm::uint parentIndex;
    float     absAngleNorm; // Current absolute angle of the branch
    float relRestAngleNorm; // Branch tries to stay in this relative angle to parent
    float     angleVelNorm;
    float     radiusTi;
    float     lengthTi;
    float     density;
    float     stiffness; // Resistance to bending
    glm::vec2 padding;
};

} // namespace rw