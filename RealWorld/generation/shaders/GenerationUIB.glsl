layout(std140, binding = 0) uniform GenerationUIB {
    ivec2 chunkOffsetTi;
    int seed;
    uint edgeConsolidationCycle;
    ivec2 edgeConsolidationThresholds;
};
