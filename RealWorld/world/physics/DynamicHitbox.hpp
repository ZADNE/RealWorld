#pragma once
#include <RealWorld/world/physics/Hitbox.hpp>

class World;

class DynamicHitbox : public Hitbox{
public:
	DynamicHitbox(const glm::ivec2& botLeftPx, const glm::ivec2& dimensionPx, const glm::ivec2& offsetPx = glm::ivec2(0, 0));
	DynamicHitbox(const glm::ivec2& botLeftPx, const glm::ivec2& dimensionPx, World* world, const glm::ivec2& offsetPx = glm::ivec2(0, 0));
	~DynamicHitbox();

	//Setters
	void setPosition(const glm::ivec2& positionPx) override;
	void setPositionX(int positionPx) override;
	void setPositionY(int positionPx) override;

	void setVelocity(const glm::vec2& velocityPx);
	void setVelocityX(float velocityPx);
	void setVelocityY(float velocityPx);

	void setFriction(const glm::vec2& frictionPx);
	//Adders
	void addVelocity(const glm::vec2& velocityPx);
	void addVelocityX(float velocityPx);
	void addVelocityY(float velocityPx);
	//Limiters
	void limitVelocity(const glm::vec2& velocityPx);
	void limitVelocityX(float velocityPx);
	void limitVelocityY(float velocityPx);

	//Getters
	glm::vec2 getVelocity() const;
	glm::vec2 getFriction() const;

	//Should be called every physics step (not draw step)
	void step() override;
private:
	//Basic physics variables
	glm::vec2 m_velocityPx = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_frictionPx = glm::vec2(0.0f, 0.0f);//Removed from velocity each step

	bool m_justChanged = false;
};