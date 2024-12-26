/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/constants/world.glsl>
#include <RealWorld/drawing/shaders/WorldDrawingPC.glsl>

layout (location = 0) out vec4  o_color;

layout (set = 0, binding = 0) uniform usampler2DArray u_worldTex;
layout (set = 0, binding = 1) uniform sampler2D       u_blockAtlas;
layout (set = 0, binding = 2) uniform sampler2D       u_wallAtlas;

layout (location = 0) in vec2   i_offsetPx;

void main() {
    // Calculate positions and offsets
    ivec2 pTi = p_.botLeftTi + pxToTi(ivec2(i_offsetPx));

    // Fetch the tile
    ivec2 pAt = tiToAt(pTi, p_.worldTexMask);
    uvec2 block = texelFetch(u_worldTex, ivec3(pAt, k_blockLayer), 0).xy & 0xff;
    uvec2 wall = texelFetch(u_worldTex, ivec3(pAt, k_wallLayer), 0).xy   & 0xff;

    // Fetch color of the block and wall
    vec4 blockColor = texelFetch(u_blockAtlas, ivec2(block.yx), 0);
    vec4 wallColor = texelFetch(u_wallAtlas, ivec2(wall.yx), 0);

    // Calculate background color
    float t = float(pTi.y - 1024) * 0.001;
    t = clamp(1.0 - t, 0.0, 1.0);
    vec3 background = p_.skyColor.rgb * t * t;

    // Blend all the colors
    vec3 temp = mix(background, wallColor.rgb, wallColor.a);
    o_color = vec4(mix(temp, blockColor.rgb, blockColor.a), 1.0);
}
