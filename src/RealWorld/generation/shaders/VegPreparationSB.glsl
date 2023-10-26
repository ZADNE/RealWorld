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
    uvec2 sizeTi;
    uint  templateIndex;// Index of the template
    uint  writeIndex;   // Index to the branch buffers (vector and raster)
    uint  randomSeed;
};

layout (binding = VegPreparationSB_BINDING, std430)
VegPreparationSB_ACCESS restrict buffer VegPreparationSB {
    uvec4           b_vegetationDispatchSize;
    uvec4           b_branchDispatchSize;
    VegInstance     b_vegInstances[32];
    BranchInstance  b_branchInstances[32*64];
};

#endif // !VEG_PREPARATION_SB_GLSL