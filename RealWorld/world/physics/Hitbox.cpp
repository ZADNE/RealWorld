#include <RealWorld/world/physics/Hitbox.hpp>

#include <algorithm>

#include <RealEngine/utility/utility.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/position_conversions.hpp>


Hitbox::Hitbox(const glm::ivec2& positionPx, const glm::ivec2& dimensionPx, const glm::ivec2& offsetPx/* = glm::ivec2(0, 0)*/) :
	p_botLeftPx(positionPx),
	//p_positionRemainder(0.0f),
	p_dimensionPx(dimensionPx/* - glm::ivec2(1, 1)*/),
	p_offsetPx(offsetPx) {

}

Hitbox::Hitbox(const glm::ivec2& positionPx, const glm::ivec2& dimensionPx, World* world, const glm::ivec2& offsetPx/* = glm::ivec2(0, 0)*/) :
	p_botLeftPx(positionPx),
	//p_positionRemainder(0.0f),
	p_dimensionPx(dimensionPx/* - glm::ivec2(1, 1)*/),
	p_world(world),
	p_offsetPx(offsetPx) {

}

Hitbox::~Hitbox() {

}

void Hitbox::setPosition(const glm::ivec2& positionPx) {
	p_botLeftPx = positionPx - p_offsetPx;
}

void Hitbox::setPositionX(int positionPx) {
	p_botLeftPx.x = positionPx - p_offsetPx.x;
}

void Hitbox::setPositionY(int positionPx) {
	p_botLeftPx.y = positionPx - p_offsetPx.y;
}

void Hitbox::setDims(const glm::ivec2& dimensionPx){
	p_dimensionPx = dimensionPx/* + glm::ivec2(1, 1)*/;
}

glm::ivec2 Hitbox::getPos() const {
	return p_botLeftPx + p_offsetPx;
}

bool Hitbox::isGrounded() const {
	glm::vec2 grav = p_world->getGravity();
	if (p_world->getMax(chunk::BLOCK_VALUES::BLOCK,
		pxToBc(p_botLeftPx + glm::ivec2(glm::sign(grav))),
		pxToBc(p_botLeftPx + glm::ivec2(glm::sign(grav)) + p_dimensionPx)) > LAST_NONSOLIDBLOCK) {
		return true;
	}
	return false;
}

bool Hitbox::collidesWith(Hitbox& otherHitbox) const {
	if (p_botLeftPx.x <= (otherHitbox.p_botLeftPx.x + otherHitbox.p_dimensionPx.x) && (p_botLeftPx.x + p_dimensionPx.x) >= otherHitbox.p_botLeftPx.x &&
		p_botLeftPx.y <= (otherHitbox.p_botLeftPx.y + otherHitbox.p_dimensionPx.y) && (p_botLeftPx.y + p_dimensionPx.y) >= otherHitbox.p_botLeftPx.y) {
		return true;
	}
	return false;
}

bool Hitbox::overlaps(const glm::ivec2& pointPx) const{
	if (p_botLeftPx.x <= pointPx.x && (p_botLeftPx.x + p_dimensionPx.x) >= pointPx.x &&
		p_botLeftPx.y <= pointPx.y && (p_botLeftPx.y + p_dimensionPx.y) >= pointPx.y) {
		return true;
	}
	return false;
}

bool Hitbox::overlapsBlockwise(const glm::ivec2& pointPx) const{
	glm::ivec2 botLeftBc = pxToBc(p_botLeftPx);
	glm::ivec2 topRightBc = pxToBc(p_botLeftPx + p_dimensionPx);
	glm::ivec2 pointBc = pxToBc(pointPx);

	if (botLeftBc.x <= pointBc.x && topRightBc.x >= pointBc.x &&
		botLeftBc.y <= pointBc.y && topRightBc.y >= pointBc.y) {
		return true;
	}
	return false;
}

void Hitbox::step() {

}