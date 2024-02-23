/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/constants/world.glsl>
#include <RealWorld/drawing/shaders/TileDrawerPC.glsl>

layout (location = 0) out vec4 o_tileColor;

layout (set = 0, binding = 0) uniform usampler2DArray u_worldTex;
layout (set = 0, binding = 1) uniform sampler2D       u_blockAtlas;
layout (set = 0, binding = 2) uniform sampler2D       u_wallAtlas;

const vec2 POS[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};


void main() {
    // Calculate positions and offsets
    ivec2 offsetTi = ivec2(gl_InstanceIndex % p_viewSizeTi.x, gl_InstanceIndex / p_viewSizeTi.x);
    vec2 posTi = POS[gl_VertexIndex] + vec2(offsetTi);

    // Fetch the tile
    ivec2 posAt = tiToAt(p_botLeftTi + offsetTi, p_worldTexMask);
    uvec2 block = texelFetch(u_worldTex, ivec3(posAt, k_blockLayer), 0).xy;
    uvec2 wall = texelFetch(u_worldTex, ivec3(posAt, k_wallLayer), 0).xy;

    // Clip if both block and wall are air
    float clip = (isAirBlock(block.L_T) && isAirWall(wall.L_T)) ? -1.0 : 1.0;

    // Output position
    gl_Position = p_viewMat * vec4(posTi * TilePx - p_botLeftPxModTilePx, 0.0, clip);

    // Calculate color of this tile based on its block and wall
    vec4 blockColor = texelFetch(u_blockAtlas, ivec2(block.yx), 0);
    vec4 wallColor = texelFetch(u_wallAtlas, ivec2(wall.yx), 0);
    o_tileColor = mix(wallColor, blockColor, blockColor.a);
}
