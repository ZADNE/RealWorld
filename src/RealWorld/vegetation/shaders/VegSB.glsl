/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_SB_GLSL
#define VEG_SB_GLSL

layout(constant_id = 0) const uint k_maxVegCount = 1;

layout (set = 0, binding = VegSB_BINDING, std430)
restrict buffer VegSB {
    uint    b_vegCount;
    uint    b_padding;
    vec2    b_rootPosTi[k_maxVegCount];
    ivec2   b_firstIndexAndCount[k_maxVegCount];
};

#endif // !VEG_SB_GLSL