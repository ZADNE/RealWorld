/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_LIGHT_GLSL
#define RW_LIGHT_GLSL
#include <RealShaders/CppIntegration.glsl>

constexpr int k_lightMaxRangeTi = 320;

constexpr int k_lightMinCellTi = 4; ///< Minimum size of light calculation cells
constexpr int k_lightMaxCellTi = 16; ///< Minimum size of light calculation cells
constexpr int k_lightMinCellTiMask = k_lightMinCellTi - 1;
constexpr int k_lightMaxCellTiMask = k_lightMaxCellTi - 1;
constexpr int k_lightMinCellTiBitShift = 2;
constexpr int k_lightMaxCellTiBitShift = 4;

/// Number of cell sizes (between min and max cell size)
constexpr int k_lightCellSizeCount = k_lightMaxCellTiBitShift - k_lightMinCellTiBitShift + 1; 

constexpr float k_analysisGroupSize = 8;
constexpr float k_lightSweepGroupSize = 8;
constexpr int k_addExternalLightsGroupSize = 8;

#endif // !RW_LIGHT_GLSL