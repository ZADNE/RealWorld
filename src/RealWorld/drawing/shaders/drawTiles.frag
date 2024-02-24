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

const vec3 k_skyBlue = vec3(0.25411764705, 0.7025490196, 0.90470588235);

void main() {
    // Calculate positions and offsets
    ivec2 offsetTi = pxToTi(ivec2(i_offsetPx));

    // Fetch the tile
    ivec2 posAt = tiToAt(p_botLeftTi + offsetTi, p_worldTexMask);
    uvec2 block = texelFetch(u_worldTex, ivec3(posAt, k_blockLayer), 0).xy;
    uvec2 wall = texelFetch(u_worldTex, ivec3(posAt, k_wallLayer), 0).xy;

    // Calculate color of this tile based on its block and wall
    vec4 blockColor = texelFetch(u_blockAtlas, ivec2(block.yx), 0);
    vec4 wallColor = texelFetch(u_wallAtlas, ivec2(wall.yx), 0);
    vec3 temp = mix(k_skyBlue.rgb, wallColor.rgb, wallColor.a);
    o_color = vec4(mix(temp, blockColor.rgb, blockColor.a), 1.0);
}
