layout(set = 0, binding = ActiveChunksSSIB_BINDING, std430) restrict buffer ActiveChunksSSIB {
    ivec2 activeChunksMask;
    ivec2 activeChunksArea;
    ivec4 dynamicsGroupSize;    //w = offset of the absolute positions in the next field
    ivec2 offsets[];            //First indexes: offsets of update chunks, in tiles
                                //Following indexes: absolute positions of chunks, in chunks
};
