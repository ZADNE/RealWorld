/*!
 *  @author     Dubsky Tomas
 */
#ifndef TREE_DYNAMICS_PC
#define TREE_DYNAMICS_PC

layout(push_constant, std430)
restrict uniform TreeDynamicsPC {
    mat4  p_mvpMat;
    vec2  p_worldTexSizeTi;
    float p_timeSec;
};

#endif // TREE_DYNAMICS_PC