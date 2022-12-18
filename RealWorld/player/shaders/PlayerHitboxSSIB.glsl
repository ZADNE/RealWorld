#include <RealWorld/reserved_units/buffers.glsl>
layout(std140, binding = STRG_BUF_PLAYER) restrict buffer PlayerHitboxSSIB {
    vec2 botLeftPx;
    vec2 dimsPx;
    vec2 velocityPx;
};
