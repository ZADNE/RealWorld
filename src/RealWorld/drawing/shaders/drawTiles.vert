/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/drawing/shaders/TileDrawerPC.glsl>

layout (location = 1) out vec2   o_offsetPx;

void main() {
    vec2 p = vec2(gl_VertexIndex & 1, gl_VertexIndex >> 1);
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);

    o_offsetPx = p * p_viewSizePx + p_botLeftPxModTilePx;
}
