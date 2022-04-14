﻿#pragma once
#include <RealWorld/physics/Hitbox.hpp>

#include <RealWorld/world/World.hpp>

class DynamicHitbox : public Hitbox {
public:
	DynamicHitbox(World* world, const glm::ivec2& botLeftPx, const glm::ivec2& dimsPx, const glm::ivec2& offsetPx = glm::ivec2(0));
	~DynamicHitbox();

	glm::vec2& velocity();
	const glm::vec2& getVelocity() const;

	void step();

	bool isGrounded() const;

	bool overlapsBlocks(const glm::ivec2& offsetPx) const;
private:
	World* p_world = nullptr;

	//Basic physics variables
	glm::vec2 m_velocityPx = glm::vec2(0);
};