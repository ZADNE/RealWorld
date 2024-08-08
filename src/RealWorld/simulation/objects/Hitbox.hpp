/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Represents an axis-aligned rectangle that can detect collisions
 */
class Hitbox {
public:
    Hitbox(glm::vec2 botLeftPx, glm::vec2 dimsPx);

    glm::vec2& botLeftPx() { return m_botLeftPx; }
    glm::vec2 botLeftPx() const { return m_botLeftPx; }

    glm::vec2& dimsPx() { return m_dimsPx; }
    glm::vec2 dimsPx() const { return m_dimsPx; }

    bool collidesWith(Hitbox& otherHitbox) const;
    bool overlaps(glm::vec2 pointPx) const;

    bool overlapsBlockwise(glm::vec2 pointPx) const;

private:
    glm::vec2 m_botLeftPx; // Bottom-left corner of the hitbox
    glm::vec2 m_dimsPx;
};

} // namespace rw
