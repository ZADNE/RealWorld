/*!
 *  @author     Dubsky Tomas
 */
layout(std430, binding = PlayerHitboxSB_BINDING)
restrict buffer PlayerHitboxSB {
    vec2 b_botLeftPx[2];
    vec2 b_dimsPx;
    vec2 b_velocityPx;
};
