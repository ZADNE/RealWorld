#include <RealWorld/world/physics/DynamicHitbox.hpp>

#include <algorithm>

#include <RealEngine/utility/utility.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/position_conversions.hpp>


DynamicHitbox::DynamicHitbox(World* world, const glm::ivec2& botLeftPx, const glm::ivec2& dimsPx, const glm::ivec2& offsetPx/* = glm::ivec2(0)*/) :
	Hitbox(botLeftPx, dimsPx - glm::ivec2(1), offsetPx),
	p_world(world),
	m_velocityPx(0) {

}

DynamicHitbox::~DynamicHitbox() {

}

glm::ivec2& DynamicHitbox::velocity() {
	return m_velocityPx;
}

const glm::ivec2& DynamicHitbox::getVelocity() const {
	return m_velocityPx;
}

void DynamicHitbox::step() {
	//Gravity
	if (!isGrounded()) {
		m_gravityCounter++;
		if (!(m_gravityCounter % p_world->addGravityEveryNSteps())) {
			m_velocityPx.y -= 1;
		}
	}

	if (overlapsBlocks({m_velocityPx.x, 0})) {
		int velSign = rmath::sign(m_velocityPx.x);
		while (!overlapsBlocks({velSign, 0})) {
			p_botLeftPx.x += velSign;
		}
		m_velocityPx.x = 0;
	}
	p_botLeftPx.x += m_velocityPx.x;

	if (overlapsBlocks({0, m_velocityPx.y})) {
		int velSign = rmath::sign(m_velocityPx.y);
		while (!overlapsBlocks({0, velSign})) {
			p_botLeftPx.y += velSign;
		}
		m_velocityPx.y = 0;
	}
	p_botLeftPx.y += m_velocityPx.y;
}

bool DynamicHitbox::isGrounded() const {
	return overlapsBlocks({0, -1});
}

bool DynamicHitbox::overlapsBlocks(const glm::ivec2& offsetPx) const {
	glm::ivec2 botLeftTi = pxToTi(p_botLeftPx + offsetPx);
	glm::ivec2 topRightTi = pxToTi(p_botLeftPx + offsetPx + p_dimsPx);

	if (p_world->getMax(TILE_VALUE::BLOCK, botLeftTi, topRightTi) > LAST_NON_SOLID_BLOCK) {
		return true;
	} else {
		return false;
	}
}