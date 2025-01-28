/** 
 *  @author    Dubsky Tomas
 */
#ifndef RW_VEGETATION_GLSL
#define RW_VEGETATION_GLSL
#include <RealWorld/constants/Tile.glsl>

const uint k_maxBranchCount         = 65536;
const uint k_maxBranchAllocCount    = 256;
const uint k_branchRasterByteCount  = 128;


const uint k_firstWoodWl            = k_oakWoodWl;
const uint k_lastWoodWl             = k_palmWoodWl;
const uint k_firstLeafWl            = k_leafWl;
const uint k_lastLeafWl             = k_palmLeafWl;
const uint k_woodToLeafTypeOffset   = k_firstLeafWl - k_firstWoodWl;

inline bool isWoodWall(uint wallType){
    return wallType >= k_firstWoodWl && wallType <= k_lastWoodWl;
}
inline bool isLeafWall(uint wallType){
    return wallType >= k_firstLeafWl && wallType <= k_lastLeafWl;
}

#endif // !RW_VEGETATION_GLSL