/**
 *  @author    Dubsky Tomas
 */
#include <algorithm>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/simulation/objects/Hitbox.hpp>

namespace rw {

Hitbox::Hitbox(glm::vec2 botLeftPx, glm::vec2 dimsPx)
    : m_botLeftPx(botLeftPx)
    , m_dimsPx(dimsPx) {
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

bool Hitbox::overlaps(glm::vec2 pointPx) const {
    if (m_botLeftPx.x <= pointPx.x && (m_botLeftPx.x + m_dimsPx.x) >= pointPx.x &&
        m_botLeftPx.y <= pointPx.y && (m_botLeftPx.y + m_dimsPx.y) >= pointPx.y) {
        return true;
    }
    return false;
}

bool Hitbox::overlapsBlockwise(glm::vec2 pointPx) const {
    glm::vec2 botLeftTi  = pxToTi(m_botLeftPx);
    glm::vec2 topRightTi = pxToTi(m_botLeftPx + m_dimsPx);
    glm::vec2 pointTi    = pxToTi(pointPx);

    if (botLeftTi.x <= pointTi.x && topRightTi.x >= pointTi.x &&
        botLeftTi.y <= pointTi.y && topRightTi.y >= pointTi.y) {
        return true;
    }
    return false;
}

} // namespace rw
