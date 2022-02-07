#include <RealWorld/world/physics/DynamicHitbox.hpp>

#include <algorithm>

#include <RealEngine/utility/utility.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/position_conversions.hpp>


DynamicHitbox::DynamicHitbox(World* world, const glm::vec2& botLeftPx, const glm::vec2& dimsPx, const glm::vec2& offsetPx/* = glm::vec2(0.0f)*/) :
	Hitbox(botLeftPx, dimsPx - glm::vec2(1.0f), offsetPx),
	p_world(world),
	m_velocityPx(0) {

}

DynamicHitbox::~DynamicHitbox() {

}

glm::vec2& DynamicHitbox::velocity() {
	return m_velocityPx;
}

const glm::vec2& DynamicHitbox::getVelocity() const {
	return m_velocityPx;
}

void DynamicHitbox::setFriction(const glm::vec2& frictionPx) {
	m_frictionPx = frictionPx;
}

glm::vec2 DynamicHitbox::getFriction() const {
	return m_frictionPx;
}

void DynamicHitbox::step() {
	if (overlapsBlocks(m_velocityPx)) {
		//Obstacle in the way - have to do precise collision checking
		glm::vec2 velNorm = glm::normalize(m_velocityPx);//Normalized velocity
		float velLength = glm::length(m_velocityPx);

		float step = glm::fract(velLength);
		velLength = glm::floor(velLength);

		//Do precise collision checking in the velocity's direction
		while (step <= velLength) {
			if (overlapsBlocks(velNorm * step)) {
				int stoppedBy;
				stoppedBy = overlapsBlocks(velNorm * step - glm::vec2(velNorm.x, 0.0f)) ? 1 : 0;
				m_velocityPx[stoppedBy] = 0.0f;
				p_botLeftPx[stoppedBy] += velNorm[stoppedBy] * (step - 1.0f);

				//Finish by sliding the other component
				velNorm[stoppedBy] = 0.0f;
				int slideBy = stoppedBy ? 0 : 1;
				while (step <= velLength) {
					if (overlapsBlocks(velNorm * step)) {
						m_velocityPx[slideBy] = 0.0f;
						p_botLeftPx[slideBy] += velNorm[slideBy] * (step - 1.0f);
						goto collisionCheckingFinished;
					}
					step++;
				}
				p_botLeftPx[slideBy] += m_velocityPx[slideBy];
				break;
			}
			step++;
		}
	} else {
		//Move without precise collision checking
		p_botLeftPx += m_velocityPx;
	}

collisionCheckingFinished:
	//Gravity
	if (!isGrounded()) {
		m_velocityPx += p_world->getGravity();
	}

	//Friction
	if (std::abs(m_velocityPx.x) > m_frictionPx.x) {
		m_velocityPx.x -= m_frictionPx.x * glm::sign(m_velocityPx.x);
	} else {
		m_velocityPx.x = 0.0f;
	}
}

bool DynamicHitbox::isGrounded() const {
	return overlapsBlocks(glm::sign(p_world->getGravity()));
}

bool DynamicHitbox::overlapsBlocks(const glm::vec2& offsetPx) const {
	glm::ivec2 botLeftTi = glm::ivec2(pxToTi(p_botLeftPx + offsetPx));
	glm::ivec2 topRightTi = glm::ivec2(pxToTi(p_botLeftPx + offsetPx + p_dimsPx));

	if (p_world->getMax(TILE_VALUE::BLOCK, botLeftTi, topRightTi) > LAST_NONSOLIDBLOCK) {
		return true;
	} else {
		return false;
	}
}

glm::uvec2 DynamicHitbox::dimsTi() const {
	return glm::uvec2(glm::ceil(p_dimsPx / TILE_SIZE));
}
