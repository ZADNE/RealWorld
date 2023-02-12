/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/drawing/shaders/TileDrawerPC.glsl>

layout(location = 0) out vec2 o_minimapUV;


const vec2 c_uv[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main() {
    gl_Position = p_viewMat * vec4(p_minimapOffset + c_uv[gl_VertexIndex] * p_minimapSize, 0.0, 1.0);
    o_minimapUV = c_uv[gl_VertexIndex];
}
