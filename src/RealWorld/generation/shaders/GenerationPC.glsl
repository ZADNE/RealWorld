/*!
 *  @author     Dubsky Tomas
 */
layout(std430, push_constant) uniform GenerationPC {
    ivec2   p_chunkOffsetTi;
    int     p_seed;
    uint    p_storeLayer;               //Refers to layers of tile image
    uint    p_edgeConsolidationPromote;
    uint    p_edgeConsolidationReduce;
};
