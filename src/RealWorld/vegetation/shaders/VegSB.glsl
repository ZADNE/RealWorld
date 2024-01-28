/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_SB_GLSL
#define VEG_SB_GLSL
#include <RealWorld/constants/vegetation.glsl>

layout (set = 0, binding = VegSB_BINDING, std430)
restrict buffer VegSB {
    uint    vegCount;
    uint    padding;
    ivec2   rootPosCh[k_maxVegCount];
    ivec2   firstIndexAndCount[k_maxVegCount];
} b_veg;

#endif // !VEG_SB_GLSL