/*!
 *  @author     Dubsky Tomas
 */
#extension GL_EXT_scalar_block_layout : require
layout(std430, push_constant)
uniform PlayerMovementPC {
    float   p_acceleration;
    float   p_maxWalkVelocity;
    float   p_jumpVelocity;
    float   p_walkDirection;
    float   p_jump;
    float   p_autojump;
    int     p_writeIndex;// Selects PlayerHitboxSB::botLeftPx, swings every step
};
