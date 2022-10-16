#version 460
#include <RealWorld/constants/generation.glsl>
#include <RealWorld/constants/tile.glsl>
#include <RealWorld/generation/ChunkUIB.glsl>

layout(location = 0) out vec2 inChunkPosTi;

const vec2 NDC_CORNERS[] = {
    {-1.0, -1.0},
    {+1.0, -1.0},
    {-1.0, +1.0},
    {+1.0, +1.0}
};

const vec2 TILE_CORNERS[] = {
    {-GEN_BORDER_WIDTH, -GEN_BORDER_WIDTH},
    {CHUNK_SIZE.x + GEN_BORDER_WIDTH, -GEN_BORDER_WIDTH},
    {-GEN_BORDER_WIDTH, CHUNK_SIZE.y + GEN_BORDER_WIDTH},
    {CHUNK_SIZE.x + GEN_BORDER_WIDTH, CHUNK_SIZE.y + GEN_BORDER_WIDTH}
};

void main() {

}
