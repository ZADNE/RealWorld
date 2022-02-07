#pragma once
#include <glm/vec2.hpp>

class Hitbox {
public:
	Hitbox(const glm::vec2& botLeftPx, const glm::vec2& dimsPx, const glm::vec2& centerOffsetPx = glm::vec2(0.0f));
	~Hitbox();

	glm::vec2& botLeft();
	const glm::vec2& getBotLeft() const;

	void setDims(const glm::vec2& dimensionPx);

	glm::vec2 getCenter() const;
	bool collidesWith(Hitbox& otherHitbox) const;
	bool overlaps(const glm::vec2& pointPx) const;

	bool overlapsBlockwise(const glm::vec2& pointPx) const;
protected:
	//Basic physics variables
	glm::vec2 p_botLeftPx;//Bottom-left corner of the hitbox
	glm::vec2 p_dimsPx;
	glm::vec2 p_centerOffsetPx;//m_botLeftPx + m_offsetPx -> considered as center of the hitbox
};