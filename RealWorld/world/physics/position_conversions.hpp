#pragma once
#include <glm/vec2.hpp>

#include <RealWorld/constants/tile.hpp>

/**
 * @brief Converts position in pixels to position in tiles
 *
 * @param posPx Position in pixels to convert
 * @return Position in blocks
 */
inline glm::vec2 pxToTi(const glm::vec2& posPx) {
	return glm::floor(posPx / TILEPx);
}

/**
 * @brief Converts position in tiles to position in pixels
 *
 * @param posTi Position in tiles to convert
 * @return Position in pixels
 */
inline glm::vec2 tiToPx(const glm::vec2& posTi) {
	return posTi * TILEPx;
}