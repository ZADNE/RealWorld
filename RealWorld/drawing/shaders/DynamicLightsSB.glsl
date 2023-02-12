/*!
 *  @author     Dubsky Tomas
 */
struct DynamicLight{
    ivec2 posPx;
    uint col;
    uint padding;
};

layout(set = 0, binding = DynamicLightsSB_BINDING, std430) readonly restrict buffer DynamicLightsSB {
    DynamicLight s_lights[];
};
