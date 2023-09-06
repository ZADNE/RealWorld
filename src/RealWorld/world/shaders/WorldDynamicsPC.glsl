/*!
 *  @author     Dubsky Tomas
 */
#ifndef WORLD_DYNAMICS_PC_GLSL
#define WORLD_DYNAMICS_PC_GLSL

layout(push_constant, std430)
restrict uniform WorldDynamicsPC {
    ivec2   p_globalPosTi;
    uint    p_modifyTarget;
    uint    p_modifyShape;
    uvec2   p_modifySetValue;
    float   p_modifyRadius;
    uint    p_timeHash;
    uint    p_updateOrder;
};

#endif // WORLD_DYNAMICS_PC_GLSL