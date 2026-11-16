/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_PLAYER_MOVEMENT_PC_GLSL
#define RW_PLAYER_MOVEMENT_PC_GLSL
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
    int     writeIndex;///< Selects PlayerHitboxSB::botLeftPx, swings every step
} RE_GLSL_ONLY(p_);

#endif // !RW_PLAYER_MOVEMENT_PC_GLSL