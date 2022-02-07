R""(
layout(location = 0) in vec2 basePosUn;
layout(location = 2) in vec2 baseUV;

flat out vec4 fColor;

layout(std140, binding = 0) uniform ViewportMatrices {
	mat4 viewMat;
};

layout(binding = TEX_UNIT_LIGHT_FINISHED) uniform sampler2D lightTexture;
layout(location = 3) uniform vec2 offsetPx;
uniform int viewWidthUn;

void main() {
	vec2 posUn = basePosUn + vec2(gl_InstanceID % viewWidthUn, gl_InstanceID / viewWidthUn);
	
	gl_Position = viewMat * vec4(posUn * TILEPx * LIGHT_DOWNSAMPLE - offsetPx, 0.0, 1.0);
	
	fColor = texelFetch(lightTexture,
				ivec2(posUn.x + LIGHT_MAX_RANGEUn, textureSize(lightTexture, 0).y - posUn.y - 1 - LIGHT_MAX_RANGEUn), 0);
}

)""