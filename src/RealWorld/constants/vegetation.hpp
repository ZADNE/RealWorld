/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <cstdint>

namespace rw {

/**
 * @brief Is the maximum number of vegetation that can be simulated
 */
constexpr uint32_t k_maxVegCount = 64 * 64 * 2;

/**
 * @brief Is the maximum number of branches that can be simulated
 */
constexpr uint32_t k_maxBranchCount = 64 * 64 * 16;

/**
 * @brief Is the number of bytes in raster buffer that each branch has
 */
constexpr uint32_t k_branchRasterSpace = 64;

} // namespace rw
