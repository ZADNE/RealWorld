#pragma once
#include <RealWorld/constants/tile.hpp>

/**
 * @brief Dimensions of chunks, in tiles
 *
 * All chunks have the same dimensions.
*/
const glm::ivec2 CHUNK_SIZE = glm::ivec2(128, 128);

const int ACTIVE_CHUNKS_AREA_X = 16;
const int ACTIVE_CHUNKS_AREA_Y = ACTIVE_CHUNKS_AREA_X;
const int ACTIVE_CHUNKS_MAX_UPDATES = (ACTIVE_CHUNKS_AREA_X - 1) * (ACTIVE_CHUNKS_AREA_Y - 1);

const glm::ivec2 ACTIVE_CHUNKS_AREA = glm::ivec2(ACTIVE_CHUNKS_AREA_X, ACTIVE_CHUNKS_AREA_Y);