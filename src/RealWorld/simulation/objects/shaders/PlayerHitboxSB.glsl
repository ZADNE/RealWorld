/**
 *  @author     Dubsky Tomas
 */
#ifndef PLAYER_HITBOX_SB_GLSL
#define PLAYER_HITBOX_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (binding = k_playerBinding, scalar)
restrict buffer PlayerHitboxSB {
    vec2 botLeftPx[2];
    vec2 dimsPx;
    vec2 velocityPx;
} RE_GLSL_ONLY(b_player);

#endif // !PLAYER_HITBOX_SB_GLSL