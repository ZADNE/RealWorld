#include <RealWorld/physics/Hitbox.hpp>

#include <algorithm>

#include <RealEngine/utility/utility.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/TDB.hpp>


Hitbox::Hitbox(const glm::ivec2& botLeftPx, const glm::ivec2& dimsPx, const glm::ivec2& centerOffsetPx/* = glm::ivec2(0.0f)*/) :
	p_botLeftPx(botLeftPx),
	p_dimsPx(dimsPx),
	p_centerOffsetPx(centerOffsetPx) {

}

Hitbox::~Hitbox() {

}

glm::ivec2& Hitbox::botLeft() {
	return p_botLeftPx;
}

const glm::ivec2& Hitbox::getBotLeft() const {
	return p_botLeftPx;
}

void Hitbox::setDims(const glm::ivec2& dimensionPx) {
	p_dimsPx = dimensionPx;
}

glm::ivec2 Hitbox::getCenter() const {
	return p_botLeftPx + p_centerOffsetPx;
}

bool Hitbox::collidesWith(Hitbox& otherHitbox) const {
	if (p_botLeftPx.x <= (otherHitbox.p_botLeftPx.x + otherHitbox.p_dimsPx.x) && (p_botLeftPx.x + p_dimsPx.x) >= otherHitbox.p_botLeftPx.x &&
		p_botLeftPx.y <= (otherHitbox.p_botLeftPx.y + otherHitbox.p_dimsPx.y) && (p_botLeftPx.y + p_dimsPx.y) >= otherHitbox.p_botLeftPx.y) {
		return true;
	}
	return false;
}

bool Hitbox::overlaps(const glm::ivec2& pointPx) const {
	if (p_botLeftPx.x <= pointPx.x && (p_botLeftPx.x + p_dimsPx.x) >= pointPx.x &&
		p_botLeftPx.y <= pointPx.y && (p_botLeftPx.y + p_dimsPx.y) >= pointPx.y) {
		return true;
	}
	return false;
}

bool Hitbox::overlapsBlockwise(const glm::ivec2& pointPx) const {
	glm::ivec2 botLeftTi = pxToTi(p_botLeftPx);
	glm::ivec2 topRightTi = pxToTi(p_botLeftPx + p_dimsPx);
	glm::ivec2 pointTi = pxToTi(pointPx);

	if (botLeftTi.x <= pointTi.x && topRightTi.x >= pointTi.x &&
		botLeftTi.y <= pointTi.y && topRightTi.y >= pointTi.y) {
		return true;
	}
	return false;
}