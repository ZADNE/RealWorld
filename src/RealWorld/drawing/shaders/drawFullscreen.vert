/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/drawing/shaders/WorldDrawingPC.glsl>

layout (location = 0) out vec2   o_uv;

void main() {
    vec2 p = vec2(gl_VertexIndex & 1, gl_VertexIndex >> 1) * 2.0;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);

    o_uv = p * p_uvRectSize + p_uvRectOffset;
}
