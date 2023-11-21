/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Represents a rectangle that can detect collisions with other Hitboxes.
 */
class Hitbox {
public:
    Hitbox(
        glm::ivec2 botLeftPx,
        glm::ivec2 dimsPx,
        glm::ivec2 centerOffsetPx = glm::ivec2(0, 0)
    );

    glm::ivec2& botLeft();
    glm::ivec2  botLeft() const;

    void setDims(glm::ivec2 dimensionPx);

    glm::ivec2 center() const;
    bool       collidesWith(Hitbox& otherHitbox) const;
    bool       overlaps(glm::ivec2 pointPx) const;

    bool overlapsBlockwise(glm::ivec2 pointPx) const;

protected:
    glm::ivec2 m_botLeftPx; // Bottom-left corner of the hitbox
    glm::ivec2 m_dimsPx;
    glm::ivec2 m_centerOffsetPx; // m_botLeftPx + m_offsetPx -> considered as
                                 // center of the hitbox
};

} // namespace rw