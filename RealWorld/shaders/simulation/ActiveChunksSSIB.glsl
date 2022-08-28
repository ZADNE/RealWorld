R""(
layout(std430, binding = 0) restrict buffer ActiveChunksSSIB {
    ivec4 dynamicsGroupSize;    //w = offset of the absolute positions in the next field
    ivec2 offsets[];            //First indexes: offsets of update chunks, in tiles
                                //Following indexes: absolute positions of chunks, in chunks
};

)""