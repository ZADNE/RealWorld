/*!
 *  @author     Dubsky Tomas
 */
#ifndef TILE_LOAD_GLSL
#define TILE_LOAD_GLSL

layout (set = 0, binding = worldImage_BINDING, rgba8ui)
uniform restrict readonly uimage2D u_worldImage;

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

//Loads tile from the given IMAGE position
uvec4 tileLoadIm(ivec2 posIm){
    return imageLoad(u_worldImage, posIm);
}

#endif // !TILE_LOAD_GLSL