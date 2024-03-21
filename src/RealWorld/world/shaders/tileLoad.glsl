/*!
 *  @author     Dubsky Tomas
 */
#ifndef TILE_LOAD_GLSL
#define TILE_LOAD_GLSL
#include <RealWorld/constants/world.glsl>

layout (set = 0, binding = k_worldTexBinding, rg8ui)
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

#endif // !TILE_LOAD_GLSL