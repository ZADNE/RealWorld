R""(
#version 460

layout(location = 0) in vec2 position;

out vec2 pos;

layout(std140) uniform ChunkUniforms {
	mat4 chunkGenMatrix;
	vec2 chunkOffsetBc;
	vec2 chunkDims;
	vec2 chunkBorders;
	int seed;
};

void main() {
	gl_Position = chunkGenMatrix * vec4(position, 0.0, 1.0);
	pos = chunkOffsetBc + vec2(position.x, chunkDims.y - position.y);
}

)""