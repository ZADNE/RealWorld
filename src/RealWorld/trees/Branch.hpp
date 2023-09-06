/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief   Is a solid section of a tree
 * @details A branch is always attached to end of a parent branch.
 *          The root branch is parent of itself.
 *          Trees may only bend in pivots that connect these sections.
 * @see     TreeSimulator
 */
struct Branch {
    /**
     * @brief Holds tightly packed angles related to the branch
     */
    struct Angles {
        uint8_t absAngleNorm;     // Current absolute angle of the branch
        uint8_t relRestAngleNorm; // Relative angle to parent that the branch is
                                  // trying to keep
        uint8_t angleVelNorm;     // Current angular velocity of the angle
        uint8_t unused{0};
    };
    static_assert(sizeof(Angles) == sizeof(glm::uint));

    glm::vec2    absPosTi; // Position of the end that connected to parent
    unsigned int parentIndex;
    Angles       angles;
    float        radiusTi;
    float        lengthTi;
    float        density;
    float        stiffness; // Resistance to bending
};

} // namespace rw
