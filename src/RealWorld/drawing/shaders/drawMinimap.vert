/**
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/drawing/shaders/WorldDrawingPC.glsl>

layout (location = 0) out vec2 o_minimapUV;

void main() {
    vec2 p = vec2(gl_VertexIndex & 1, gl_VertexIndex >> 1);
    gl_Position = p_.minimapViewMat * vec4(p_.minimapOffset + p * p_.minimapSize, 0.0, 1.0);
    o_minimapUV = p;
}
