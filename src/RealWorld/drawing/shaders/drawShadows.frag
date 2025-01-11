/**
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec4  o_color;

layout (set = 0, binding = 0) uniform sampler2D u_shadowsSampler;

layout (location = 0) in vec2   i_uv;

void main() {
    o_color = texture(u_shadowsSampler, i_uv);
}
