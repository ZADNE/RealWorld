/*! 
 *  @author    Dubsky Tomas
 */
#ifndef WORLD_GLSL
#define WORLD_GLSL
#include <RealWorld/constants/chunk.glsl>

const uvec2 k_minWorldTexSizeCh = uvec2(16, 16);
const uvec2 k_maxWorldTexSizeCh = uvec2(64, 64);
const uint  k_maxWorldTexChunkCount = k_maxWorldTexSizeCh.x * k_maxWorldTexSizeCh.y;
const uint  k_chunkTransferSlots = 16;

const uint k_blockLayer = 0;
const uint k_wallLayer = 1;

ivec2 tiToAt(ivec2 posTi, ivec2 worldTexMaskTi){
    return ivec2(posTi & worldTexMaskTi);
}

ivec2 chToAc(ivec2 posCh, ivec2 worldTexSizeCh) {
    return ivec2(posCh & (worldTexSizeCh - 1));
}

int chToIndex(ivec2 posCh, ivec2 worldTexSizeCh) {
    ivec2 posAc = chToAc(posCh, worldTexSizeCh);
    return posAc.y * worldTexSizeCh.x + posAc.x;
}

#endif // !WORLD_GLSL