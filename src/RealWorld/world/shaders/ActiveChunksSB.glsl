/*!
 *  @author     Dubsky Tomas
 */
#ifndef ACTIVE_CHUNKS_SB_GLSL
#define ACTIVE_CHUNKS_SB_GLSL

layout (set = 0, binding = k_acChunksBinding, std430)
restrict buffer ActiveChunksSB {
    ivec2 worldTexSizeMask;
    ivec2 worldTexSizeCh;
    ivec4 dynamicsGroupSize;  // w = offset of the absolute positions in the next field
    ivec2 offsets[];          // First indices: offsets of update chunks, in tiles
                                // Following indices: absolute positions of chunks, in chunks
} b_acChunks;

int g_maxNumberOfUpdateChunks =
    b_acChunks.worldTexSizeMask.x * b_acChunks.worldTexSizeMask.y;

ivec2 activeChunkAtPos(ivec2 posAc){
    return b_acChunks.offsets[g_maxNumberOfUpdateChunks +
                     posAc.y * b_acChunks.worldTexSizeCh.x + posAc.x];
}

#endif // !ACTIVE_CHUNKS_SB_GLSL