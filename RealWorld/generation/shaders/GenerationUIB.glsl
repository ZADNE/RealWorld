layout(std430, push_constant) uniform GenerationUIB {
    ivec2 chunkOffsetTi;
    int seed;
    uint edgeConsolidationCycle;
    ivec2 edgeConsolidationThresholds;
};
