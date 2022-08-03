/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/physics/Hitbox.hpp>

#include <algorithm>

#include <RealWorld/world/World.hpp>


Hitbox::Hitbox(const glm::ivec2& botLeftPx, const glm::ivec2& dimsPx, const glm::ivec2& centerOffsetPx/* = glm::ivec2(0.0f)*/) :
	m_botLeftPx(botLeftPx),
	m_dimsPx(dimsPx),
	m_centerOffsetPx(centerOffsetPx) {

}

Hitbox::~Hitbox() {

}

glm::ivec2& Hitbox::botLeft() {
	return m_botLeftPx;
}

const glm::ivec2& Hitbox::getBotLeft() const {
	return m_botLeftPx;
}

void Hitbox::setDims(const glm::ivec2& dimensionPx) {
	m_dimsPx = dimensionPx;
}

glm::ivec2 Hitbox::getCenter() const {
	return m_botLeftPx + m_centerOffsetPx;
}

bool Hitbox::collidesWith(Hitbox& otherHitbox) const {
	if (m_botLeftPx.x <= (otherHitbox.m_botLeftPx.x + otherHitbox.m_dimsPx.x) && (m_botLeftPx.x + m_dimsPx.x) >= otherHitbox.m_botLeftPx.x &&
		m_botLeftPx.y <= (otherHitbox.m_botLeftPx.y + otherHitbox.m_dimsPx.y) && (m_botLeftPx.y + m_dimsPx.y) >= otherHitbox.m_botLeftPx.y) {
		return true;
	}
	return false;
}

bool Hitbox::overlaps(const glm::ivec2& pointPx) const {
	if (m_botLeftPx.x <= pointPx.x && (m_botLeftPx.x + m_dimsPx.x) >= pointPx.x &&
		m_botLeftPx.y <= pointPx.y && (m_botLeftPx.y + m_dimsPx.y) >= pointPx.y) {
		return true;
	}
	return false;
}

bool Hitbox::overlapsBlockwise(const glm::ivec2& pointPx) const {
	glm::ivec2 botLeftTi = pxToTi(m_botLeftPx);
	glm::ivec2 topRightTi = pxToTi(m_botLeftPx + m_dimsPx);
	glm::ivec2 pointTi = pxToTi(pointPx);

	if (botLeftTi.x <= pointTi.x && topRightTi.x >= pointTi.x &&
		botLeftTi.y <= pointTi.y && topRightTi.y >= pointTi.y) {
		return true;
	}
	return false;
}