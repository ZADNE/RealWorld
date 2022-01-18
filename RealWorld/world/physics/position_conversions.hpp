#pragma once
#include <glm/vec2.hpp>

#include <RealWorld/metadata.hpp>

/**
 * Converts position in pixels to position in blocks.
 * 
 * \param posPx Position in pixels to convert
 * \return Position in blocks
 */
inline glm::ivec2 pxToBc(glm::ivec2 posPx) {
    div_t x = div(posPx.x, ivec2_BLOCK_SIZE.x);
    div_t y = div(posPx.y, ivec2_BLOCK_SIZE.y);

    glm::ivec2 r = { x.quot, y.quot };
	if (x.rem != 0 && posPx.x < 0) r.x--;
	if (y.rem != 0 && posPx.y < 0) r.y--;
	return r;
}

/**
 * Converts position in blocks to position in pixels.
 *
 * \param posBc Position in blocks to convert
 * \return Position in pixels
 */
inline glm::ivec2 bcToPx(glm::ivec2 posBc) {
	return posBc * ivec2_BLOCK_SIZE;
}