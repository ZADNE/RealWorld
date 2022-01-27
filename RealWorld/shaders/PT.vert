R""(
#version 460

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 UV;

layout(std140) uniform WorldDrawUniforms {
	mat4 viewMat;
	mat4 viewsizePxMat;
	mat4 viewsizeLightingBcMat;
};

void main() {
	gl_Position = viewsizeLightingBcMat * vec4(position, 0.0, 1.0);
}

)""