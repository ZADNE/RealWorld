R""(
out vec4 fColor;

layout(binding = TEX_UNIT_SHADOWS) uniform sampler2D shadowsSampler;
layout(location = 3) uniform vec2 offsetPx;

const vec2 POS[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main() {
	ivec2 offsetTi = ivec2(gl_InstanceID % viewWidthTi, gl_InstanceID / viewWidthTi);
	vec2 posTi = POS[gl_VertexID] + vec2(offsetTi);
	
	gl_Position = viewMat * vec4(posTi * TILEPx - offsetPx, 0.0, 1.0);
	
	fColor = texelFetch(shadowsSampler, offsetTi, 0);
}

)""