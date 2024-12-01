/*!
 *  @author     Dubsky Tomas
 */
#ifndef PLAYER_MOVEMENT_PC_GLSL
#define PLAYER_MOVEMENT_PC_GLSL

#include <RealShaders/CppIntegration.glsl>

layout (push_constant, scalar)
uniform PlayerMovementPC {
    ivec2   worldTexMaskTi;
    float   acceleration;
    float   maxWalkVelocity;
    float   jumpVelocity;
    float   walkDirection;
    float   jump;
    float   autojump;
    int     writeIndex;// Selects PlayerHitboxSB::botLeftPx, swings every step
} RE_SHADER_INSTANCE(p_);

#endif // !PLAYER_MOVEMENT_PC_GLSL