/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_TILE_LOAD_GLSL
#define RW_TILE_LOAD_GLSL
#include <RealWorld/constants/World.glsl>

layout (set = 0, binding = k_worldTexBinding, rg16ui)
uniform restrict readonly uimage2DArray u_worldImage;

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

#endif // !RW_TILE_LOAD_GLSL