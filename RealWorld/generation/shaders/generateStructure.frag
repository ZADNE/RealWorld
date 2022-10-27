#version 460
#include <RealWorld/generation/shaders/generateStructure.glsl>

layout(location = 0) in vec2 inChunkPosTi;

layout(location = 0) out uvec4 tile;
layout(location = 1) out uvec4 material;

void main(){
    vec2 pPx = (chunkOffsetTi + floor(inChunkPosTi)) * TILEPx;
    basicTerrain(pPx, material, tile);
}