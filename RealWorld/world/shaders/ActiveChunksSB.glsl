/*!
 *  @author     Dubsky Tomas
 */
layout(set = 0, binding = ActiveChunksSB_BINDING, std430) restrict buffer ActiveChunksSB {
    ivec2 b_worldTexSizeMask;
    ivec2 b_worldTexSizeCh;
    ivec4 b_dynamicsGroupSize;  //w = offset of the absolute positions in the next field
    ivec2 b_offsets[];          //First indexes: offsets of update chunks, in tiles
                                //Following indexes: absolute positions of chunks, in chunks
};
