/*! 
 *  @author    Dubsky Tomas
 */
#ifndef VEGETATION_GLSL
#define VEGETATION_GLSL
#include <RealWorld/constants/tile.glsl>

const uint k_maxBranchCount         = 65536;
const uint k_maxBranchAllocCount    = 256;
const uint k_branchRasterSpace      = 64;


const uint k_firstWoodWl            = k_oakWoodWl;
const uint k_lastWoodWl             = k_coniferousWoodWl;
const uint k_firstLeafWl            = k_leafWl;
const uint k_lastLeafWl             = k_needleWl;
const uint k_woodToLeafTypeOffset   = k_firstLeafWl - k_firstWoodWl;

bool isWoodWall(uint wallType){
    return wallType >= k_firstWoodWl && wallType <= k_lastWoodWl;
}
bool isLeafWall(uint wallType){
    return wallType >= k_firstLeafWl && wallType <= k_lastLeafWl;
}

#endif // !VEGETATION_GLSL