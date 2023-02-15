/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <bit>

#include <RealWorld/constants/tile.hpp>

/**
 * @brief Dimensions of a chunk, in tiles
 *
 * All chunks have the same dimensions.
 * Both componenets must be powers of 2.
*/
constexpr glm::uvec2 uCHUNK_DIMS = glm::uvec2(128u, 128u);
constexpr glm::ivec2 iCHUNK_DIMS = uCHUNK_DIMS;
constexpr glm::vec2 CHUNK_DIMS = uCHUNK_DIMS;

static_assert(std::has_single_bit(uCHUNK_DIMS.x) && std::has_single_bit(uCHUNK_DIMS.y));

/**
 * @brief Size of a chunk in bytes
*/
constexpr size_t CHUNK_BYTE_SIZE = static_cast<size_t>(iCHUNK_DIMS.x) * iCHUNK_DIMS.y * 4;

/**
 * @brief Converts a position in tiles to position in chunks
*/
constexpr inline glm::ivec2 tiToCh(const glm::ivec2& posTi) {
    constexpr glm::ivec2 CHUNK_LOW_ZERO_BITS = glm::ivec2(std::countr_zero(uCHUNK_DIMS.x), std::countr_zero(uCHUNK_DIMS.x));
    return posTi >> CHUNK_LOW_ZERO_BITS;
}

/**
 * @brief Converts a position in chunks to position in tiles
*/
constexpr inline glm::ivec2 chToTi(const glm::ivec2& posCh) {
    constexpr glm::ivec2 CHUNK_LOW_ZERO_BITS = glm::ivec2(std::countr_zero(uCHUNK_DIMS.x), std::countr_zero(uCHUNK_DIMS.x));
    return posCh << CHUNK_LOW_ZERO_BITS;
}

/**
 * @brief Converts a position in chunks to its active form equivalent
*/
constexpr inline glm::ivec2 chToAc(const glm::ivec2& posCh, const glm::ivec2& activeChunksMask) {
    return posCh & activeChunksMask;
}

/**
 * @brief Converts a position in chunks to its active form measured in tiles
*/
constexpr inline glm::ivec2 chToAt(const glm::ivec2& posCh, const glm::ivec2& activeChunksMask) {
    return chToTi(chToAc(posCh, activeChunksMask));
}

constexpr inline int acToIndex(const glm::ivec2& posAc, const glm::ivec2& activeChunks) {
    return posAc.y * activeChunks.x + posAc.x;
}
