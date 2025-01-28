/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_WORLD_DYNAMICS_PC_GLSL
#define RW_WORLD_DYNAMICS_PC_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (push_constant, scalar)
restrict uniform WorldDynamicsPC {
    ivec2   playerPosTi;
    ivec2   globalOffsetTi;
    ivec2   worldTexMaskTi;
    uint    modifyLayer;
    uint    modifyShape;
    uvec2   modifySetValue;
    int     modifyMaxCount;
    float   modifyRadius;
    uint    timeHash;
    uint    updateOrder;
    float   timeSec;
} RE_GLSL_ONLY(p_);

#endif // !RW_WORLD_DYNAMICS_PC_GLSL