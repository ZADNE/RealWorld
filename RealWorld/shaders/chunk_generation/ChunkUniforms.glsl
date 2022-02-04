R""(
layout(std140, binding = 0) uniform ChunkUniforms {
	mat4 chunkGenMatrix;
	vec2 chunkOffsetTi;
	vec2 chunkDimsTi;
	vec2 chunkBordersTi;
	float seed;
};

)""