layout(std430, push_constant) uniform GenerationPC {
    ivec2 chunkOffsetTi;
    int seed;
    uint storeLayer;                //Refers to layers of tile image
    uint edgeConsolidationPromote;
    uint edgeConsolidationReduce;
};
