/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/constants/tile.glsl>

const int k_analysisGroupSize = 8;

const int k_calcGroupSize = 8;

const int k_iLightScale = 4;
const int k_lightScaleBits = 2;
const float k_iLightScaleInv = 0.25;
const int k_lightMaxRangeUn = k_lightMaxRangeTi >> k_lightScaleBits;
