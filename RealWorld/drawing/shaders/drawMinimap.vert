#version 460
#include <RealWorld/drawing/shaders/WorldDrawingPC.glsl>

layout(location = 0) out vec2 o_minimapUV;


const vec2 c_pos[4] = {{200.0, 50.0}, {1200.0, 50.0}, {200.0, 1050.0}, {1200.0, 1050.0}};
const vec2 c_uv[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main() {
    gl_Position = viewMat * vec4(c_pos[gl_VertexIndex], 0.0, 1.0);
    o_minimapUV = c_uv[gl_VertexIndex];
}
