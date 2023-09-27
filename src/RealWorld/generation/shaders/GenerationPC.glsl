/*!
 *  @author     Dubsky Tomas
 */
#ifndef GENERATION_PC_GLSL
#define GENERATION_PC_GLSL

layout(push_constant, std430)
uniform GenerationPC {
    ivec2   p_chunkOffsetTi;
    int     p_seed;
    uint    p_storeLayer;               //Refers to layers of tile image
    uint    p_edgeConsolidationPromote;
    uint    p_edgeConsolidationReduce;
};

#endif // !GENERATION_PC_GLSL