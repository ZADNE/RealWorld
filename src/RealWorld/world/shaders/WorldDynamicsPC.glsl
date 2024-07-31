/*!
 *  @author     Dubsky Tomas
 */
#ifndef WORLD_DYNAMICS_PC_GLSL
#define WORLD_DYNAMICS_PC_GLSL

layout (push_constant, std430)
restrict uniform WorldDynamicsPC {
    ivec2   p_globalOffsetTi;
    ivec2   p_worldTexMaskTi;
    uint    p_modifyLayer;
    uint    p_modifyShape;
    uvec2   p_modifySetValue;
    int     p_modifyMaxCount;
    float   p_modifyRadius;
    uint    p_timeHash;
    uint    p_updateOrder;
    float   p_timeSec;
};

#endif // !WORLD_DYNAMICS_PC_GLSL