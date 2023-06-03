/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/chunk.hpp>

/**
 * @brief Width of the padding around the generated chunk.
 *
 * Avoids errors at edges of generated chunks.
 */
constexpr int k_genBorderWidth = 8;

/**
 * @brief Size of the area that is generated for each chunk
 *
 * It is bigger than the actual chunk because it contains the padding around.
 *
 * @see k_genBorderWidth
 */
constexpr glm::ivec2 k_genChunkSize = iChunkTi + 2 * k_genBorderWidth;
