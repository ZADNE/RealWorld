R""(
layout(location = 0) in vec2 basePosTi;
layout(location = 2) in vec2 baseUV;

out vec4 fColor;

layout(std140, binding = 0) uniform ViewportMatrices {
	mat4 viewMat;
};

layout(binding = TEX_UNIT_SHADOWS) uniform sampler2D shadowsSampler;
layout(location = 3) uniform vec2 offsetPx;
uniform int viewWidthTi;

void main() {
	ivec2 offsetTi = ivec2(gl_InstanceID % viewWidthTi, gl_InstanceID / viewWidthTi);
	vec2 posTi = basePosTi + vec2(offsetTi);
	
	gl_Position = viewMat * vec4(posTi * TILEPx - offsetPx, 0.0, 1.0);
	
	fColor = texelFetch(shadowsSampler, offsetTi, 0);
}

)""