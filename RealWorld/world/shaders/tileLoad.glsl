layout(binding = worldImage_BINDING, rgba8ui) uniform restrict readonly uimage2D worldImage;

//Converts world position to image position
ivec2 imPos(ivec2 posTi){
    return posTi & (imageSize(worldImage) - 1);
}
//Converts 2 world positions to 2 image positions
ivec4 imPos(ivec4 posTi){
    return posTi & (imageSize(worldImage).xyxy - 1);
}

//Loads tile from the given WORLD position
uvec4 tileLoad(ivec2 posTi){
    return imageLoad(worldImage, imPos(posTi));
}

//Loads tile from the given IMAGE position
uvec4 tileLoadIm(ivec2 posIm){
    return imageLoad(worldImage, posIm);
}
