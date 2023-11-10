/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_DYNAMICS_PC_GLSL
#define VEG_DYNAMICS_PC_GLSL

layout (push_constant, std430)
restrict uniform VegDynamicsPC {
    mat4  p_mvpMat;
    vec2  p_worldTexSizeTi;
    float p_timeSec;
    uint  p_readBuf;
};

#endif // !VEG_DYNAMICS_PC_GLSL