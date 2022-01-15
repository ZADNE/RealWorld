R""(
#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in float direction;
layout(location = 3) in float cone;

out vec4 fragColour;
out float lightDir;
out float lightCone;
out vec2 interpolation;

layout(std140) uniform WorldDrawUniforms {
	mat4 viewMat;
	mat4 viewsizePxMat;
	mat4 viewsizeLightingBcMat;
};

layout(location = 3) uniform vec2 botLeftBc;
uniform vec2 blockSizePx;

void main() {
	vec2 pos = (position + blockSizePx * 0.5) / blockSizePx - botLeftBc;
	vec2 blockPos;
	interpolation = modf(pos, blockPos);
	
	gl_Position = viewsizeLightingBcMat * vec4(blockPos, 0.0, 1.0);
	gl_PointSize = 2.0;
	fragColour = colour;
	lightDir = direction;
	lightCone = cone;
}

)""