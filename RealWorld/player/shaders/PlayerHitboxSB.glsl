/*!
 *  @author     Dubsky Tomas
 */
layout(std430, binding = PlayerHitboxSB_BINDING)
restrict buffer PlayerHitboxSB {
    vec2 s_botLeftPx[2];
    vec2 s_dimsPx;
    vec2 s_velocityPx;
};
