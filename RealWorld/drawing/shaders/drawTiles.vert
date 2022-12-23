#version 460
#include <RealWorld/reserved_units/textures.glsl>
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/drawing/shaders/WorldDrawerUIB.glsl>

layout(location = 0) out vec4 tileColor;

layout(binding = TEX_UNIT_WORLD_TEXTURE) uniform usampler2D worldTexture;
layout(binding = TEX_UNIT_BLOCK_ATLAS) uniform sampler2D blockTexture;
layout(binding = TEX_UNIT_WALL_ATLAS) uniform sampler2D wallTexture;

const vec2 POS[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};


void main() {
    //Calculate positions and offsets
    ivec2 offsetTi = ivec2(gl_InstanceIndex % viewWidthTi, gl_InstanceIndex / viewWidthTi);
    vec2 posTi = POS[gl_VertexIndex] + vec2(offsetTi);
    
    //Fetch the tile
    uvec4 tile = texelFetch(worldTexture, (botLeftTi + offsetTi) & worldTexMask, 0);
    
    //Clip if both block and wall are air
    float clip = (isAirBlock(tile.BLOCK_TYPE) && isAirWall(tile.WALL_TYPE)) ? -1.0 : 1.0;
    
    //Output position
    gl_Position = viewMat * vec4(posTi * TILEPx - botLeftPxModTilePx, 0.0, clip);
    
    //Calculate color of this tile based on its block and wall
    vec4 blockColor = texelFetch(blockTexture, ivec2(tile.yx), 0);
    vec4 wallColor = texelFetch(wallTexture, ivec2(tile.wz), 0);
    tileColor = mix(wallColor, blockColor, blockColor.a);
}
