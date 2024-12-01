/*!
 *  @author     Dubsky Tomas
 */
#ifndef ACTIVE_CHUNKS_SB_GLSL
#define ACTIVE_CHUNKS_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (set = 0, binding = k_acChunksBinding, scalar)
restrict buffer ActiveChunksSB {
    ivec2 worldTexSizeMask;
    ivec2 worldTexSizeCh;
    ivec4 dynamicsGroupSize; // w = offset of the absolute positions in the next field
    ivec2 offsets[]; // First indices: offsets of update chunks, in tiles
    // Following indices: absolute positions of chunks, in chunks
} RE_SHADER_INSTANCE(b_acChunks);

#ifdef VULKAN

int g_maxNumberOfUpdateChunks =
    b_acChunks.worldTexSizeMask.x * b_acChunks.worldTexSizeMask.y;

ivec2 activeChunkAtPos(ivec2 posAc){
    return b_acChunks.offsets[g_maxNumberOfUpdateChunks +
                     posAc.y * b_acChunks.worldTexSizeCh.x + posAc.x];
}

#endif

#endif // !ACTIVE_CHUNKS_SB_GLSL