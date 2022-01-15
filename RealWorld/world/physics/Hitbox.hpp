#pragma once
#include <GL\glew.h>
#include <glm\vec2.hpp>

class World;

class Hitbox {
public:
	Hitbox(const glm::ivec2& botLeftPx, const glm::ivec2& dimensionPx, const glm::ivec2& offsetPx = glm::ivec2(0, 0));
	Hitbox(const glm::ivec2& botLeftPx, const glm::ivec2& dimensionPx, World* world, const glm::ivec2& offsetPx = glm::ivec2(0, 0));
	~Hitbox();

	void init(World* world) {
		if (p_world == nullptr) {
			p_world = world;
		}
	};

	//Setters
	virtual void setPosition(const glm::ivec2& positionPx);
	virtual void setPositionX(int positionPx);
	virtual void setPositionY(int positionPx);

	void setDims(const glm::ivec2& dimensionPx);
	//Getters
		
		//Origin of the hitbox
	glm::ivec2 getPos() const;
	bool isGrounded() const;
	bool collidesWith(Hitbox& otherHitbox) const;
	bool overlaps(const glm::ivec2& pointPx) const;

	bool overlapsBlockwise(const glm::ivec2& pointPx) const;

	//Should be called every physics step (not draw step)
	virtual void step();
protected:
	//Basic physics variables
	glm::ivec2 p_botLeftPx;//Bottom-left corner of the hitbox
	//glm::vec2 m_positionRemainder;
	glm::ivec2 p_dimensionPx;
	glm::ivec2 p_offsetPx;//m_botLeftPx + m_offsetPx -> considered as center of the hitbox

	World* p_world = nullptr;
};