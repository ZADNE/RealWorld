/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/Chunk.hpp>

namespace rw {

/**
 * @brief Width of the padding around the generated chunk.
 * @details Avoids errors at edges of generated chunks.
 */
constexpr int k_genBorderWidth = 8;

/**
 * @brief Size of the area that is generated for each chunk
 * @details It is bigger than the actual chunk because it contains the padding around.
 * @see k_genBorderWidth
 */
constexpr glm::ivec2 k_genChunkSize = iChunkTi + 2 * k_genBorderWidth;

/**
 * @brief Is the maximum number of chunks that can be generated in parallel
 */
constexpr int k_chunkGenSlots = 12;

/**
 * @brief Is the maximum number of vegetation instances that can be generated in
 * a chunk
 */
constexpr int k_maxVegPerChunk = 64;

/**
 * @brief Is the number of branches that can be prepared during generation
 */
constexpr int k_branchGenSlots = k_chunkGenSlots * 256;

constexpr int k_lSystemSpeciesCount = 6;

} // namespace rw
