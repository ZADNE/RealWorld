/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Is the maximum number of branches that can be simulated
 * @warning Going above 65536 needs refactoring (it is packed to 16 bits in some
 * places)
 */
constexpr uint32_t k_maxBranchCount = 65536;

/**
 * @brief Maximum number of individual allocations with the branch buffer
 */
constexpr uint32_t k_maxBranchAllocCount = 256;

/**
 * @brief Is the number of bytes in raster buffer that each branch has
 */
constexpr uint32_t k_branchRasterByteCount = 128;

} // namespace rw
