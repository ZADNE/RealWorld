/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <bit>

#include <RealWorld/constants/tile.hpp>

namespace rw {

/**
 * @brief Dimensions of a chunk, in tiles
 *
 * All chunks have the same dimensions.
 * Both componenets must be powers of 2.
 */
constexpr glm::uvec2 uChunkTi = glm::uvec2(128u, 128u);
constexpr glm::ivec2 iChunkTi = uChunkTi;
constexpr glm::vec2  ChunkTi  = uChunkTi;

static_assert(std::has_single_bit(uChunkTi.x) && std::has_single_bit(uChunkTi.y));

/**
 * @brief Size of a chunk in bytes
 */
constexpr size_t k_chunkByteSize = static_cast<size_t>(iChunkTi.x) * iChunkTi.y * 4;

constexpr glm::ivec2 k_chunkLowZeroBits =
    glm::ivec2(std::countr_zero(uChunkTi.x), std::countr_zero(uChunkTi.y));

/**
 * @brief Converts a position in tiles to position in chunks
 */
constexpr inline glm::ivec2 tiToCh(const glm::ivec2& posTi) {
    return posTi >> k_chunkLowZeroBits;
}

/**
 * @brief Converts a position in chunks to position in tiles
 */
constexpr inline glm::ivec2 chToTi(const glm::ivec2& posCh) {
    return posCh << k_chunkLowZeroBits;
}

/**
 * @brief Converts a position in chunks to its active form equivalent
 */
constexpr inline glm::ivec2 chToAc(
    const glm::ivec2& posCh, const glm::ivec2& worldTexSizeMask
) {
    return posCh & worldTexSizeMask;
}

/**
 * @brief Converts a position in chunks to its active form measured in tiles
 */
constexpr inline glm::ivec2 chToAt(
    const glm::ivec2& posCh, const glm::ivec2& worldTexSizeMask
) {
    return chToTi(chToAc(posCh, worldTexSizeMask));
}

constexpr inline int acToIndex(const glm::ivec2& posAc, const glm::ivec2& activeChunks) {
    return posAc.y * activeChunks.x + posAc.x;
}

} // namespace rw
