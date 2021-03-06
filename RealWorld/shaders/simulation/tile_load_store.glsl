R""(
layout(binding = IMG_UNIT_WORLD, rgba8ui) uniform restrict coherent uimage2D worldImage;

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

//Stores tile to the given WORLD position
void tileStore(ivec2 posTi, uvec4 tile){
	imageStore(worldImage, imPos(posTi), tile);
}

//Loads tile from the given IMAGE position
uvec4 tileLoadIm(ivec2 posIm){
	return imageLoad(worldImage, posIm);
}

//Stores tile to the given IMAGE position
void tileStoreIm(ivec2 posIm, uvec4 tile){
	imageStore(worldImage, posIm, tile);
}

)""