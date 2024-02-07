/*! 
 *  @author    Dubsky Tomas
 */
#ifndef VEGETATION_GLSL
#define VEGETATION_GLSL

const uint  k_maxBranchCount = 65536;
const uint  k_maxBranchAllocCount = 256;
const uint  k_branchRasterSpace = 64;
const uvec2 k_maxWorldTexSizeCh = uvec2(64, 64);
const uint  k_maxWorldTexChunkCount = k_maxWorldTexSizeCh.x * k_maxWorldTexSizeCh.y;

#endif // !VEGETATION_GLSL