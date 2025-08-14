/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <bit>

namespace rw {

constexpr float k_analysisGroupSize = 8;
constexpr float k_calcGroupSize     = 8;

constexpr int k_lightMaxRangeTi = 160;

constexpr int k_lightMinCellTi = 2; ///< Minimum size of light calculation cells
constexpr int k_lightMaxCellTi = 16; ///< Minimum size of light calculation cells
constexpr int k_lightMinCellTiBits = 0b1;
constexpr int k_lightMaxCellTiBits = 0b1111;

constexpr int k_lightCellTiCount = 4;

static_assert(k_lightMinCellTiBits + 1 == k_lightMinCellTi);
static_assert(k_lightMaxCellTiBits + 1 == k_lightMaxCellTi);
static_assert(
    std::popcount(glm::uint{k_lightMaxCellTiBits - k_lightMinCellTiBits}) + 1 ==
    k_lightCellTiCount
);

} // namespace rw
