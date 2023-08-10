/*!
 *  @author     Dubsky Tomas
 */
layout(push_constant, std430) restrict uniform TreeDynamicsPC {
    mat4  p_mvpMat;
    vec2  p_worldTexSizeTi;
    float p_timeSec;
};
