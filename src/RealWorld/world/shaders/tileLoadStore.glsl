/*!
 *  @author     Dubsky Tomas
 */
#ifndef TILE_LOAD_STORE_GLSL
#define TILE_LOAD_STORE_GLSL
#include <RealWorld/constants/world.glsl>

layout (set = 0, binding = k_worldTexBinding, rg8ui)
uniform restrict coherent uimage2DArray u_worldImage;

uvec2 loadLayer(ivec2 posAt, uint layer){
    return imageLoad(u_worldImage, ivec3(posAt, layer)).xy;
}

uvec2 loadBlock(ivec2 posAt){
    return loadLayer(posAt, k_blockLayer);
}

uvec2 loadWall(ivec2 posAt){
    return loadLayer(posAt, k_wallLayer);
}

uvec4 loadTile(ivec2 posAt){
    return uvec4(loadBlock(posAt), loadWall(posAt));
}

void storeLayer(ivec2 posAt, uint layer, uvec2 typeVar){
    imageStore(u_worldImage, ivec3(posAt, layer), uvec4(typeVar, 0, 0));
}

void storeBlock(ivec2 posAt, uvec2 block){
    storeLayer(posAt, k_blockLayer, block);
}

void storeWall(ivec2 posAt, uvec2 wall){
    storeLayer(posAt, k_wallLayer, wall);
}

void storeTile(ivec2 posAt, uvec4 tile){
    storeBlock(posAt, tile.rg);
    storeWall(posAt, tile.ba);
}

#endif // !TILE_LOAD_STORE_GLSL