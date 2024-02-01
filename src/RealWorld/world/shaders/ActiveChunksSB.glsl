/*!
 *  @author     Dubsky Tomas
 */
#ifndef ACTIVE_CHUNKS_SB_GLSL
#define ACTIVE_CHUNKS_SB_GLSL

layout (set = 0, binding = ActiveChunksSB_BINDING, std430)
restrict buffer ActiveChunksSB {
    ivec2 b_worldTexSizeMask;
    ivec2 b_worldTexSizeCh;
    ivec4 b_dynamicsGroupSize;  // w = offset of the absolute positions in the next field
    ivec2 b_offsets[];          // First indices: offsets of update chunks, in tiles
                                // Following indices: absolute positions of chunks, in chunks
};

int g_maxNumberOfUpdateChunks = b_worldTexSizeMask.x * b_worldTexSizeMask.y;

ivec2 activeChunkAtPos(ivec2 posAc){
    return b_offsets[g_maxNumberOfUpdateChunks + posAc.y * b_worldTexSizeCh.x + posAc.x];
}

#endif // !ACTIVE_CHUNKS_SB_GLSL