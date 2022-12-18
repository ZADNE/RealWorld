#include <RealWorld/reserved_units/buffers.glsl>
layout(std430, binding = STRG_BUF_ACTIVECHUNKS) restrict buffer ActiveChunksSSIB {
    ivec2 activeChunksMask;
    ivec2 activeChunksArea;
    ivec4 dynamicsGroupSize;    //w = offset of the absolute positions in the next field
    ivec2 offsets[];            //First indexes: offsets of update chunks, in tiles
                                //Following indexes: absolute positions of chunks, in chunks
};
