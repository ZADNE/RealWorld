/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/world.glsl>

layout (location = 0) out vec4   o_tileColor;

layout (location = 0) in  vec2   i_minimapUV;

layout (set = 0, binding = 0) uniform usampler2DArray u_worldSampler;
layout (set = 0, binding = 1) uniform sampler2D       u_blockAtlas;
layout (set = 0, binding = 2) uniform sampler2D       u_wallAtlas;

void main() {
    // Fetch the tile
    uvec2 block = texture(u_worldSampler, vec3(i_minimapUV, k_blockLayer), 0).xy;
    uvec2 wall = texture(u_worldSampler, vec3(i_minimapUV, k_wallLayer), 0).xy;

    // Calculate color of this tile based on its block and wall color
    vec4 blockColor = texelFetch(u_blockAtlas, ivec2(block.yx), 0);
    vec4 wallColor = texelFetch(u_wallAtlas, ivec2(wall.yx), 0);
    o_tileColor = mix(wallColor, blockColor, blockColor.a);
}
