/*!
 *  @author     Dubsky Tomas
 */
#ifndef GENERATION_PC_GLSL
#define GENERATION_PC_GLSL

layout (push_constant, std430)
uniform GenerationPC {
    ivec2 p_chunkTi[k_maxParallelChunks];
    ivec2 p_worldTexSizeCh;
    int   p_seed;
    uint  p_storeSegment;
    uint  p_edgeConsolidationPromote;
    uint  p_edgeConsolidationReduce;
    uint  p_branchWriteBuf;
};

uint tileImgLoadLayer(){
    return (1 - p_storeSegment) * k_maxParallelChunks + gl_WorkGroupID.z;
}

uint tileImgStoreLayer(){
    return p_storeSegment * k_maxParallelChunks + gl_WorkGroupID.z;
}

#endif // !GENERATION_PC_GLSL