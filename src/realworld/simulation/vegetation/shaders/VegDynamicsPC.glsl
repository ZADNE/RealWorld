/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_VEG_DYNAMICS_PC_GLSL
#define RW_VEG_DYNAMICS_PC_GLSL
#include <realshaders/CppIntegration.glsl>

layout (push_constant, scalar)
restrict uniform VegDynamicsPC {
    mat4  mvpMat;
    vec2  worldTexSizeTi;
    ivec2 worldTexMaskTi;
    float timeSec;
    uint  readBuf;
} RE_GLSL_ONLY(p_);

#endif // !RW_VEG_DYNAMICS_PC_GLSL