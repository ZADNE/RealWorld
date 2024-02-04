/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_PREPARATION_SB_GLSL
#define VEG_PREPARATION_SB_GLSL

#ifndef VegPreparationSB_ACCESS
#define VegPreparationSB_ACCESS
#endif

struct VegInstance{
    uint  templateIndex;// Index of the template
    uint  writeIndex;   // Index to the branch buffers (vector and raster)
    uint  randomSeed;
    uint  branchBaseIndex;// Index into b_branchInstances
    vec2  rootPosTi;
    float sizeFactor;
    float angleFactor;
};

struct BranchInstance{
    vec2  offsetTi;
    uvec2 sizeTi;
    uint  wallType;
    uint  writeIndex;   // Index to the branch buffers (vector and raster)
    uint  randomSeed;
};

layout (binding = VegPreparationSB_BINDING, std430)
VegPreparationSB_ACCESS restrict buffer VegPreparationSB {
    uvec4           vegDispatchSize;
    uvec4           branchDispatchSize;
    VegInstance     vegInstances[32];
    BranchInstance  branchInstances[32*64];
} b_vegPrep;

#endif // !VEG_PREPARATION_SB_GLSL