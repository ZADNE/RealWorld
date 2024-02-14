/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_PREP_SB_GLSL
#define VEG_PREP_SB_GLSL
#include <RealWorld/constants/generation.glsl>

struct VegInstance{
    uint  templateIndex; // Index of the template
    uint  outputIndex;   // Index to the branch buffers (vector and raster)
    uint  randomSeed;
    uint  branchPrepInstIndex;// Index into b_vegPrep.branchInstances
    vec2  rootPosTi;
    float sizeFactor;
    float angleFactor;
};

struct BranchInstance{
    vec2  offsetTi;
    uvec2 sizeTi;
    uint  wallType;
    uint  outputIndex;   // Index to the branch buffers (vector and raster)
    uint  randomSeed;
};

#ifndef VegPrepSB_ACCESS
#define VegPrepSB_ACCESS
#endif

layout (binding = k_vegPrepBinding, std430)
VegPrepSB_ACCESS restrict buffer VegPrepSB {
    uvec4           vegDispatchSize;
    uvec4           branchDispatchSize;
    VegInstance     vegInstances[k_chunkGenSlots * 64];
    BranchInstance  branchInstances[k_chunkGenSlots * 64 * 64];
} b_vegPrep;

#endif // !VEG_PREP_SB_GLSL