/*!
 *  @author     Dubsky Tomas
 */
#ifndef VEG_DYNAMICS_PC
#define VEG_DYNAMICS_PC

layout (push_constant, std430)
restrict uniform VegDynamicsPC {
    mat4  p_mvpMat;
    vec2  p_worldTexSizeTi;
    float p_timeSec;
};

#endif // !VEG_DYNAMICS_PC