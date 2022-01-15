R""(
#version 440

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 UV;

out vec2 fragUV;

layout(std140) uniform WorldDrawUniforms {
	mat4 viewMat;
	mat4 viewsizePxMat;
	mat4 viewsizeLightingBcMat;
	ivec2 worldOffsetBc;
};

layout(location = 3) uniform vec2 offset;//pos % BLOCK_SIZE

void main() {
	gl_Position = viewsizePxMat * vec4(position - offset, 0.0, 1.0);
	fragUV = UV;
}

)""