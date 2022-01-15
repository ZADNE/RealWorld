R""(
#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in float direction;
layout(location = 3) in float cone;

out vec4 fragColour;
out float lightDir;
out float lightCone;

layout(std140) uniform WorldDrawUniforms {
	mat4 viewMat;
	mat4 viewsizePxMat;
	mat4 viewsizeLightingBcMat;
	ivec2 worldOffsetBc;
};

layout(location = 3) uniform ivec2 botLeftBc;

void main() {
	gl_Position = viewsizeLightingBcMat * vec4(position - vec2(botLeftBc), 0.0, 1.0);
	fragColour = colour;
	lightDir = direction;
	lightCone = cone;
}

)""