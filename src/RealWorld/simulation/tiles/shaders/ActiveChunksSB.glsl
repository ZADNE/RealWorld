/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_ACTIVE_CHUNKS_SB_GLSL
#define RW_ACTIVE_CHUNKS_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (set = 0, binding = k_acChunksBinding, scalar)
restrict buffer ActiveChunksSB {
    ivec2 worldTexSizeMask;
    ivec2 worldTexSizeCh;
    ivec4 dynamicsGroupSize; // w = offset of the absolute positions in the next field
    ivec2 offsets[]; // First indices: offsets of update chunks, in tiles
    // The following indices: absolute positions of chunks, in chunks
} RE_GLSL_ONLY(b_acChunks);

#ifdef VULKAN

int g_maxNumberOfUpdateChunks =
    b_acChunks.worldTexSizeMask.x * b_acChunks.worldTexSizeMask.y;

ivec2 activeChunkAtPos(ivec2 posAc){
    return b_acChunks.offsets[g_maxNumberOfUpdateChunks +
                     posAc.y * b_acChunks.worldTexSizeCh.x + posAc.x];
}

#endif

#endif // !RW_ACTIVE_CHUNKS_SB_GLSL