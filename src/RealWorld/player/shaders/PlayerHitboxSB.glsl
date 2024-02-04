/*!
 *  @author     Dubsky Tomas
 */
#ifndef PLAYER_HITBOX_SB_GLSL
#define PLAYER_HITBOX_SB_GLSL

layout (binding = PlayerHitboxSB_BINDING, std430)
restrict buffer PlayerHitboxSB {
    vec2 botLeftPx[2];
    vec2 dimsPx;
    vec2 velocityPx;
} b_player;

#endif // !PLAYER_HITBOX_SB_GLSL