/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/vec2.hpp>

#include <RealWorld/constants/chunk.hpp>

namespace rw {
/**
 * @brief Is the minimum size of the world texture, in chunks
 */
constexpr glm::ivec2 k_minWorldTexSizeCh{16, 16};

/**
 * @brief Is the maximum size of the world texture, in chunks
 */
constexpr glm::ivec2 k_maxWorldTexSizeCh{64, 64};

/**
 * @brief Is the maximum number of chunks of the world texture
 */
constexpr uint32_t k_maxWorldTexChunkCount = k_maxWorldTexSizeCh.x *
                                             k_maxWorldTexSizeCh.y;

/**
 * @brief Is the maximum number of parallel uploads and/or downloads within a step
 */
constexpr uint32_t k_chunkTransferSlots = 16;

/**
 * @brief Converts a position in chunks to its active form equivalent
 */
constexpr glm::ivec2 chToAc(glm::ivec2 posCh, glm::ivec2 worldTexMaskCh) {
    return posCh & worldTexMaskCh;
}

/**
 * @brief Converts a position in chunks to its active form measured in tiles
 */
constexpr glm::ivec2 chToAt(glm::ivec2 posCh, glm::ivec2 worldTexMaskCh) {
    return chToTi(chToAc(posCh, worldTexMaskCh));
}

constexpr int acToIndex(glm::ivec2 posAc, glm::ivec2 worldTexCh) {
    return posAc.y * worldTexCh.x + posAc.x;
}

} // namespace rw
