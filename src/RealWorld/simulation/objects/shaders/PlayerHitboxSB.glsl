/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_PLAYER_HITBOX_SB_GLSL
#define RW_PLAYER_HITBOX_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (binding = k_playerBinding, scalar)
restrict buffer PlayerHitboxSB {
    vec2 botLeftPx[2];
    vec2 dimsPx;
    vec2 velocityPx;
} RE_GLSL_ONLY(b_player);

#endif // !RW_PLAYER_HITBOX_SB_GLSL