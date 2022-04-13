R""(
layout(location = 0) in vec2 basePosPx;
layout(location = 1) in vec4 lightColour;
layout(location = 2) in float lightDir;
layout(location = 3) in float lightCone;

out vec4 vert_light;
out vec4 vert_diaphragm;

layout(location = 3) uniform vec2 botLeftUn;

const vec2 POS_OFFSETS[4] = vec2[4](
	vec2(-0.5, -0.5),
	vec2(+0.5, -0.5),
	vec2(-0.5, +0.5),
	vec2(+0.5, +0.5)
);

const vec2 NORM_POSITIONS[4] = vec2[4](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 1.0)
);

float lightPower(in vec2 subTileNorm, in vec2 tileNorm){
	vec2 diff = vec2(1.0) - abs(subTileNorm - tileNorm);
	return diff.x * diff.y;
}

void main() {
	vec2 basePosUn = basePosPx * (1.0 / TILEPx / LIGHT_DOWNSAMPLE);
	vec2 posUn = basePosUn - botLeftUn + POS_OFFSETS[gl_InstanceID];
	gl_Position = viewsizeLightingUnMat * vec4(posUn, 0.0, 1.0);
	
	vec2 subTileNorm = fract(basePosUn + vec2(0.5, 0.5));
	
	vert_light = lightColour * lightPower(subTileNorm, NORM_POSITIONS[gl_InstanceID]);
	vert_diaphragm = vec4(0.0, lightDir, lightCone, 0.0);
}

)""