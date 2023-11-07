/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout (location = 0) out vec4   o_tileColor;

layout (location = 0) in  vec2   i_minimapUV;

layout (set = 0, binding = 0) uniform usampler2D u_worldSampler;
layout (set = 0, binding = 1) uniform sampler2D  u_blockAtlas;
layout (set = 0, binding = 2) uniform sampler2D  u_wallAtlas;

void main() {
    //Fetch the tile
    uvec4 tile = texture(u_worldSampler, i_minimapUV);

    //Calculate color of this tile based on its block and wall color
    vec4 blockColor = texelFetch(u_blockAtlas, ivec2(tile.yx), 0);
    vec4 wallColor = texelFetch(u_wallAtlas, ivec2(tile.wz), 0);
    o_tileColor = mix(wallColor, blockColor, blockColor.a);
}
