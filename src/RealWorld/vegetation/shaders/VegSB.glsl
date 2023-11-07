/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_SB_GLSL
#define VEG_SB_GLSL

layout(constant_id = 0) const uint k_maxVegCount;

layout (set = 0, binding = VegSB_BINDING, std430)
restrict buffer VegSB {
    vec2    rootPosTi[k_maxVegCount];
    ivec2   firstIndexAndCount[k_maxVegCount];
};

#endif // !VEG_SB_GLSL