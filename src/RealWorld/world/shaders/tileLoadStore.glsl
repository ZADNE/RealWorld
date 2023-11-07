/*!
 *  @author     Dubsky Tomas
 */
#ifndef TILE_LOAD_STORE_GLSL
#define TILE_LOAD_STORE_GLSL

layout (set = 0, binding = worldImage_BINDING, rgba8ui)
uniform restrict coherent uimage2D u_worldImage;

//Converts world position to image position
ivec2 imPos(ivec2 posTi){
    return posTi & (imageSize(u_worldImage) - 1);
}
//Converts 2 world positions to 2 image positions
ivec4 imPos(ivec4 posTi){
    return posTi & (imageSize(u_worldImage).xyxy - 1);
}

//Loads tile from the given WORLD position
uvec4 tileLoad(ivec2 posTi){
    return imageLoad(u_worldImage, imPos(posTi));
}

//Stores tile to the given WORLD position
void tileStore(ivec2 posTi, uvec4 tile){
    imageStore(u_worldImage, imPos(posTi), tile);
}

//Loads tile from the given IMAGE position
uvec4 tileLoadIm(ivec2 posIm){
    return imageLoad(u_worldImage, posIm);
}

//Stores tile to the given IMAGE position
void tileStoreIm(ivec2 posIm, uvec4 tile){
    imageStore(u_worldImage, posIm, tile);
}

#endif // !TILE_LOAD_STORE_GLSL