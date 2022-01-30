R""(
#version 460

layout(location = 0) in vec2 basePosPx;
layout(location = 1) in vec4 lightColour;
layout(location = 2) in float lightDir;
layout(location = 3) in float lightCone;

out vec4 vert_light;
out vec4 vert_diaphragm;

layout(std140, binding = 0) uniform WorldDrawUniforms {
	mat4 viewsizeMatPx;
	mat4 viewsizeLightingMatBc;
};

layout(location = 3) uniform vec2 botLeftBc;
layout(location = 100) uniform sampler2D dia;
uniform vec2 invBlockSizePx;
uniform float yInversion;

const vec2 OFFSETS[4] = vec2[4](
	vec2(-0.5, -0.5),
	vec2(+0.5, -0.5),
	vec2(-0.5, +0.5),
	vec2(+0.5, +0.5)
);

const vec2 CENTERS[4] = vec2[4](
	vec2(0.5, 0.5),
	vec2(1.5, 0.5),
	vec2(0.5, 1.5),
	vec2(1.5, 1.5)
);

const vec2 OFFSETS2[4] = vec2[4](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 1.0)
);

float manhattanDistance(in vec2 a, in vec2 b){
	vec2 diff = abs(a - b);
	return (1.0 - diff.x) * (1.0 - diff.y);
}

void main() {
	vec2 basePosBc = vec2(basePosPx.x,  yInversion - basePosPx.y) * invBlockSizePx;
	vec2 posBc = basePosBc - botLeftBc + OFFSETS[gl_InstanceID];
	gl_Position = viewsizeLightingMatBc * vec4(posBc, 0.0, 1.0);
	
	vec2 subTile = fract(basePosBc + vec2(0.5, 0.5));
	float lightPower = max(manhattanDistance(subTile, OFFSETS2[gl_InstanceID]), 0.0);
	
	vert_light = lightColour * lightPower;
	vert_diaphragm = vec4(texelFetch(dia, ivec2(posBc.xy), 0).r, lightDir, lightCone, 0.0);
}

)""