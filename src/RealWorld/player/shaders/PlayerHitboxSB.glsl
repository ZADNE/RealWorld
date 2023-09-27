/*!
 *  @author     Dubsky Tomas
 */
#ifndef PLAYER_HITBOX_SB_GLSL
#define PLAYER_HITBOX_SB_GLSL

layout(binding = PlayerHitboxSB_BINDING, std430)
restrict buffer PlayerHitboxSB {
    vec2 b_botLeftPx[2];
    vec2 b_dimsPx;
    vec2 b_velocityPx;
};

#endif // !PLAYER_HITBOX_SB_GLSL