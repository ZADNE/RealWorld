/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_PREP_SB_GLSL
#define VEG_PREP_SB_GLSL
#include <RealWorld/constants/generation.glsl>

struct VegInstance{
    vec2  rootPosTi;
    uint  templateIndex;
    uint  chunkIndex;
    uint  randomSeed;
    float growth;
    float rootDirNorm;
    float tropismDirNorm;
};

layout (binding = k_vegPrepBinding, std430)
VegPrepSB_ACCESS restrict buffer VegPrepSB {
    uvec4       vegDispatchSize;
    uvec4       branchDispatchSize;
    VegInstance vegInstances[k_chunkGenSlots * k_maxVegPerChunk];
    uint        vegOffsetWithinChunk[k_chunkGenSlots * k_maxVegPerChunk];
    uint        prepIndexOfFirstBranch[k_chunkGenSlots * k_maxVegPerChunk];
    uint        branchOfChunk[k_chunkGenSlots];

    // Branches
    uint    vegIndex[k_branchGenSlots];
    vec2    absPosTi[k_branchGenSlots];
    float   absAngNorm[k_branchGenSlots];
    uint    parentOffset15wallType31[k_branchGenSlots];
    float   relRestAngNorm[k_branchGenSlots];   // rest angle relative to parent
    float   radiusTi[k_branchGenSlots];
    float   lengthTi[k_branchGenSlots];
    vec2    densityStiffness[k_branchGenSlots];
} b_vegPrep;

#endif // !VEG_PREP_SB_GLSL