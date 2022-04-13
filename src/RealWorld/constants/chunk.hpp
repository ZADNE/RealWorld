#pragma once
#include <bit>

#include <RealWorld/constants/tile.hpp>

/**
 * @brief Dimensions of chunks, in tiles
 *
 * All chunks have the same dimensions.
 * Both componenets must be powers of 2.
*/
constexpr glm::uvec2 uCHUNK_SIZE = glm::uvec2(128u, 128u);
constexpr glm::ivec2 iCHUNK_SIZE = uCHUNK_SIZE;
constexpr glm::vec2 CHUNK_SIZE = uCHUNK_SIZE;

static_assert(std::has_single_bit(uCHUNK_SIZE.x) && std::has_single_bit(uCHUNK_SIZE.y));

constexpr int ACTIVE_CHUNKS_AREA_X = 16;
constexpr int ACTIVE_CHUNKS_AREA_Y = ACTIVE_CHUNKS_AREA_X;
constexpr int ACTIVE_CHUNKS_MAX_UPDATES = (ACTIVE_CHUNKS_AREA_X - 1) * (ACTIVE_CHUNKS_AREA_Y - 1);

/**
 * @brief Size in chunks of the area that can be active (= loaded in GPU memory)
*/
constexpr glm::ivec2 ACTIVE_CHUNKS_AREA = glm::ivec2(ACTIVE_CHUNKS_AREA_X, ACTIVE_CHUNKS_AREA_Y);

static_assert(std::has_single_bit((unsigned)ACTIVE_CHUNKS_AREA.x) && std::has_single_bit((unsigned)ACTIVE_CHUNKS_AREA.y));

/**
 * @brief Converts a position in tiles to position in chunks
*/
inline glm::ivec2 tiToCh(const glm::ivec2& posTi) {
	constexpr glm::ivec2 CHUNK_LOW_ZERO_BITS = glm::ivec2(std::countr_zero(uCHUNK_SIZE.x), std::countr_zero(uCHUNK_SIZE.x));
	return posTi >> CHUNK_LOW_ZERO_BITS;
}

/**
 * @brief Converts a position in chunks to position in tiles
*/
inline glm::ivec2 chToTi(const glm::ivec2& posCh) {
	constexpr glm::ivec2 CHUNK_LOW_ZERO_BITS = glm::ivec2(std::countr_zero(uCHUNK_SIZE.x), std::countr_zero(uCHUNK_SIZE.x));
	return posCh << CHUNK_LOW_ZERO_BITS;
}

/**
 * @brief Converts a position in chunks to its active form equivalent
*/
inline glm::ivec2 chToAc(const glm::ivec2& posCh) {
	constexpr glm::ivec2 ACTIVE_CHUNKS_AREA_BITS = ACTIVE_CHUNKS_AREA - 1;
	return posCh & ACTIVE_CHUNKS_AREA_BITS;
}

/**
 * @brief Converts a position in chunks to its active form measured in tiles
*/
inline glm::ivec2 chToAt(const glm::ivec2& posCh) {
	return chToTi(chToAc(posCh));
}

inline int acToIndex(const glm::ivec2& posAc) {
	return posAc.y * ACTIVE_CHUNKS_AREA.x + posAc.x;
}
