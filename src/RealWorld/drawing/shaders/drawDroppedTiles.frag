/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>

layout (location = 0) out vec4 o_col;

layout (location = 0) in vec4  i_col;
layout (location = 1) in vec2  i_offsetPx;

void main() {
    bool border = any(lessThan(i_offsetPx, vec2(1.0))) ||
                  any(greaterThanEqual(i_offsetPx, TilePx - 1.0));
    float avg = (i_col.r + i_col.g + i_col.b) * 0.33333333;
    vec4 invertedCol = vec4(vec3(avg < 0.5), i_col.a);
    o_col = border ? invertedCol : i_col;
}
