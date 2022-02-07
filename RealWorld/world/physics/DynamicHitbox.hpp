#pragma once
#include <RealWorld/world/physics/Hitbox.hpp>

#include <RealWorld/world/World.hpp>

class DynamicHitbox : public Hitbox {
public:
	DynamicHitbox(World* world, const glm::vec2& botLeftPx, const glm::vec2& dimsPx, const glm::vec2& offsetPx = glm::vec2(0.0f));
	~DynamicHitbox();

	glm::vec2& velocity();
	const glm::vec2& getVelocity() const;

	void setFriction(const glm::vec2& frictionPx);

	glm::vec2 getFriction() const;

	void step();

	bool isGrounded() const;

	bool overlapsBlocks(const glm::vec2& offsetPx) const;
private:
	glm::uvec2 dimsTi() const;

	World* p_world = nullptr;

	//Basic physics variables
	glm::vec2 m_velocityPx = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_frictionPx = glm::vec2(0.0f, 0.0f);
};