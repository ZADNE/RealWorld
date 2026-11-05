/**
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/drawing/shaders/WorldDrawingPC.glsl>

layout (location = 0) out vec2   o_uv;
layout (location = 1) out vec2   o_viewport01; // 0 to 1 viewport coordinates

void main() {
    o_viewport01 = vec2(gl_VertexIndex & 1, gl_VertexIndex >> 1) * 2.0;
    gl_Position = vec4(o_viewport01 * 2.0 - 1.0, 0.0, 1.0);

    o_uv = o_viewport01 * p_.uvRectSize + p_.uvRectOffset;
}
