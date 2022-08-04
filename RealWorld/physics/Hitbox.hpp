/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

/**
 * @brief Represents a rectangle that can detect collisions with other Hitboxes.
*/
class Hitbox {
public:
	Hitbox(const glm::ivec2& botLeftPx, const glm::ivec2& dimsPx, const glm::ivec2& centerOffsetPx = glm::ivec2(0, 0));

	glm::ivec2& botLeft();
	const glm::ivec2& getBotLeft() const;

	void setDims(const glm::ivec2& dimensionPx);

	glm::ivec2 getCenter() const;
	bool collidesWith(Hitbox& otherHitbox) const;
	bool overlaps(const glm::ivec2& pointPx) const;

	bool overlapsBlockwise(const glm::ivec2& pointPx) const;
protected:
	//Basic physics variables
	glm::ivec2 m_botLeftPx;//Bottom-left corner of the hitbox
	glm::ivec2 m_dimsPx;
	glm::ivec2 m_centerOffsetPx;//m_botLeftPx + m_offsetPx -> considered as center of the hitbox
};