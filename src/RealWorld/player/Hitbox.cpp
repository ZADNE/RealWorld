/*!
 *  @author    Dubsky Tomas
 */
#include <algorithm>

#include <RealWorld/player/Hitbox.hpp>
#include <RealWorld/world/World.hpp>

namespace rw {

Hitbox::Hitbox(glm::ivec2 botLeftPx, glm::ivec2 dimsPx, glm::ivec2 centerOffsetPx /* = glm::ivec2(0.0f)*/)
    : m_botLeftPx(botLeftPx)
    , m_dimsPx(dimsPx)
    , m_centerOffsetPx(centerOffsetPx) {
}

glm::ivec2& Hitbox::botLeft() {
    return m_botLeftPx;
}

glm::ivec2 Hitbox::botLeft() const {
    return m_botLeftPx;
}

void Hitbox::setDims(glm::ivec2 dimensionPx) {
    m_dimsPx = dimensionPx;
}

glm::ivec2 Hitbox::center() const {
    return m_botLeftPx + m_centerOffsetPx;
}

bool Hitbox::collidesWith(Hitbox& otherHitbox) const {
    if (m_botLeftPx.x <= (otherHitbox.m_botLeftPx.x + otherHitbox.m_dimsPx.x) &&
        (m_botLeftPx.x + m_dimsPx.x) >= otherHitbox.m_botLeftPx.x &&
        m_botLeftPx.y <= (otherHitbox.m_botLeftPx.y + otherHitbox.m_dimsPx.y) &&
        (m_botLeftPx.y + m_dimsPx.y) >= otherHitbox.m_botLeftPx.y) {
        return true;
    }
    return false;
}

bool Hitbox::overlaps(glm::ivec2 pointPx) const {
    if (m_botLeftPx.x <= pointPx.x && (m_botLeftPx.x + m_dimsPx.x) >= pointPx.x &&
        m_botLeftPx.y <= pointPx.y && (m_botLeftPx.y + m_dimsPx.y) >= pointPx.y) {
        return true;
    }
    return false;
}

bool Hitbox::overlapsBlockwise(glm::ivec2 pointPx) const {
    glm::ivec2 botLeftTi  = pxToTi(m_botLeftPx);
    glm::ivec2 topRightTi = pxToTi(m_botLeftPx + m_dimsPx);
    glm::ivec2 pointTi    = pxToTi(pointPx);

    if (botLeftTi.x <= pointTi.x && topRightTi.x >= pointTi.x &&
        botLeftTi.y <= pointTi.y && topRightTi.y >= pointTi.y) {
        return true;
    }
    return false;
}

} // namespace rw
