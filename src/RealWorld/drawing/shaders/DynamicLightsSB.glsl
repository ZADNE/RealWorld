/*!
 *  @author     Dubsky Tomas
 */
#ifndef DYNAMIC_LIGHTS_SB_GLSL
#define DYNAMIC_LIGHTS_SB_GLSL

struct DynamicLight{
    ivec2 posPx;
    uint col;
    uint padding;
};

layout (set = 0, binding = DynamicLightsSB_BINDING, std430)
readonly restrict buffer DynamicLightsSB {
    DynamicLight b_lights[];
};

#endif // !DYNAMIC_LIGHTS_SB_GLSL