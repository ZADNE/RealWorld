/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_LIGHT_GLSL
#define RW_LIGHT_GLSL
#include <RealShaders/CppIntegration.glsl>

constexpr float k_analysisGroupSize = 8;
constexpr float k_calcGroupSize     = 8;

constexpr int k_lightMaxRangeTi = 160;

constexpr int k_lightMinCellTi = 2; ///< Minimum size of light calculation cells
constexpr int k_lightMaxCellTi = 16; ///< Minimum size of light calculation cells
constexpr int k_lightMinCellTiBits = 1;
constexpr int k_lightMaxCellTiBits = 15;

constexpr int k_lightCellSizeCount = 4; ///< # of cell sizes (between min and max)

#endif // !RW_LIGHT_GLSL