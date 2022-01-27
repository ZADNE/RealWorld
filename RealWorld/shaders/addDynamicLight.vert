R""(
#version 460

layout(location = 0) in vec2 posPx;
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
layout(location = 7) uniform float y;
uniform vec2 invBlockSizePx;
uniform float yInversion;

void main() {
	vec2 pos = vec2(posPx.x,  yInversion - posPx.y) * invBlockSizePx - botLeftBc;
	
	vec2 tilePosBc;
	interpolation = modf(pos, tilePosBc);
	
	gl_Position = viewsizeLightingBcMat * vec4(tilePosBc, 0.0, 1.0);
	gl_PointSize = 2.0;
	fragColour = colour;
	lightDir = direction;
	lightCone = cone;
}

)""