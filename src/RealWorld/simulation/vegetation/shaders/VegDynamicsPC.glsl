/**
 *  @author     Dubsky Tomas
 */
#ifndef VEG_DYNAMICS_PC_GLSL
#define VEG_DYNAMICS_PC_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (push_constant, scalar)
restrict uniform VegDynamicsPC {
    mat4  mvpMat;
    vec2  worldTexSizeTi;
    ivec2 worldTexMaskTi;
    float timeSec;
    uint  readBuf;
} RE_GLSL_ONLY(p_);

#endif // !VEG_DYNAMICS_PC_GLSL