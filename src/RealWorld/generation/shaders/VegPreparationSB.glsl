/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_PREPARATION_SB_GLSL
#define VEG_PREPARATION_SB_GLSL

#ifndef VegPreparationSB_ACCESS
#define VegPreparationSB_ACCESS readonly
#endif

struct VegInstance{
    uint  templateIndex;    // Index of the template
    uint  writeIndex;       // Index to the branch buffer
    uint  randomSeed;
    uint  padding;
    vec2  rootPosTi;
    float sizeFactor;
    float angleFactor;
};

layout (binding = VegPreparationSB_BINDING, std430)
VegPreparationSB_ACCESS restrict buffer VegPreparationSB {
    uvec4           b_dispatchSize;
    VegInstance     b_vegInstances[];
};

#endif // !VEG_PREPARATION_SB_GLSL