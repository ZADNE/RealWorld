R""(
layout(std430, binding = 0) buffer ActiveChunksSSBO {
	ivec2 activeChunksCh[ACTIVE_CHUNKS_AREA_X * ACTIVE_CHUNKS_AREA_Y];
	ivec2 updateOffsetTi[ACTIVE_CHUNKS_MAX_UPDATES];
	ivec4 dynamicsGroupSize;
};

)""