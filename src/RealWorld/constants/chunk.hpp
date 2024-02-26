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
constexpr glm::vec2 ChunkTi   = uChunkTi;

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
constexpr glm::vec2 tiToCh(glm::vec2 posTi) {
    glm::vec2 posChFrac = posTi / ChunkTi;
    return {std::floor(posChFrac.x), std::floor(posChFrac.y)};
}
constexpr glm::ivec2 tiToCh(glm::ivec2 posTi) {
    return posTi >> k_chunkLowZeroBits;
}

/**
 * @brief Converts a position in chunks to position in tiles
 */
constexpr glm::vec2 chToTi(glm::vec2 posCh) {
    return posCh * ChunkTi;
}
constexpr glm::ivec2 chToTi(glm::ivec2 posCh) {
    return posCh << k_chunkLowZeroBits;
}

/**
 * @brief Converts a position in pixels to position in chunks
 */
constexpr glm::vec2 pxToCh(glm::vec2 posTi) {
    glm::vec2 posChFrac = posTi / (ChunkTi * TilePx);
    return {std::floor(posChFrac.x), std::floor(posChFrac.y)};
}
constexpr glm::ivec2 pxToCh(glm::ivec2 posTi) {
    return posTi >> (k_chunkLowZeroBits + k_tileLowZeroBits);
}

/**
 * @brief Converts a position in chunks to position in pixels
 */
constexpr glm::vec2 chToPx(glm::vec2 posCh) {
    return posCh * (ChunkTi * TilePx);
}
constexpr glm::ivec2 chToPx(glm::ivec2 posCh) {
    return posCh << (k_chunkLowZeroBits + k_tileLowZeroBits);
}

} // namespace rw
