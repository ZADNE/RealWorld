/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/packing.hpp>
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
    /**
     * @brief Holds angles related to the branch
     */
    struct Angles {
        float absAngleNorm{};     // Current absolute angle of the branch
        float relRestAngleNorm{}; // Relative angle to parent that the branch is
                                  // trying to keep
        float angleVelNorm{};     // Current angular velocity of the angle
    };

    /**
     * @brief Is tightly packed representation of rw::Branch::Angles
     */
    struct PackedAngles {
        PackedAngles(const Angles& a)
            : PackedAngles{glm::packUnorm4x8(
                  {a.absAngleNorm, a.relRestAngleNorm, a.angleVelNorm, 0.0f}
              )} {}

        PackedAngles(const glm::vec4& angles)
            : PackedAngles{glm::packUnorm4x8(angles)} {}

        PackedAngles(glm::uint packed_)
            : packed(packed_) {}

        Angles unpack() const {
            glm::vec4 unpacked = glm::unpackUnorm4x8(packed);
            return Angles{unpacked.x, unpacked.y, unpacked.z};
        }

        glm::uint packed;
    };
    static_assert(sizeof(PackedAngles) == sizeof(glm::uint));

    glm::vec2    absPosTi; // Position of the end that connected to parent
    unsigned int parentIndex;
    PackedAngles angles;
    float        radiusTi;
    float        lengthTi;
    float        density;
    float        stiffness; // Resistance to bending
};

} // namespace rw
