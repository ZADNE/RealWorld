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
constexpr int GEN_BORDER_WIDTH = 8;

/**
 * @brief Size of the area that is generated for each chunk
 *
 * It is bigger than the actual chunk because it contains the padding around.
 *
 * @see GEN_BORDER_WIDTH
*/
constexpr glm::ivec2 GEN_CHUNK_SIZE = iCHUNK_SIZE + 2 * GEN_BORDER_WIDTH;
