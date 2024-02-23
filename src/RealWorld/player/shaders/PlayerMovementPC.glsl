/*!
 *  @author     Dubsky Tomas
 */
#ifndef PLAYER_MOVEMENT_PC_GLSL
#define PLAYER_MOVEMENT_PC_GLSL

#extension GL_EXT_scalar_block_layout : require
layout (push_constant, std430)
uniform PlayerMovementPC {
    ivec2   p_worldTexMaskTi;
    float   p_acceleration;
    float   p_maxWalkVelocity;
    float   p_jumpVelocity;
    float   p_walkDirection;
    float   p_jump;
    float   p_autojump;
    int     p_writeIndex;// Selects PlayerHitboxSB::botLeftPx, swings every step
};

#endif // !PLAYER_MOVEMENT_PC_GLSL