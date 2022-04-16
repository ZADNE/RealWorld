R""(
layout(location = 0) in vec2 basePosUn;
layout(location = 2) in vec2 baseUV;

out vec4 fColor;

layout(std140, binding = 0) uniform ViewportMatrices {
	mat4 viewMat;
};

layout(binding = TEX_UNIT_LIGHTS_COMPUTED) uniform sampler2D lightComputed;
layout(location = 3) uniform vec2 offsetPx;
uniform int viewWidthUn;

void main() {
	vec2 posUn = basePosUn + vec2(gl_InstanceID % viewWidthUn, gl_InstanceID / viewWidthUn);
	
	gl_Position = viewMat * vec4(posUn * TILEPx * LIGHT_DOWNSAMPLE - offsetPx, 0.0, 1.0);
	
	fColor = texture(lightComputed, (posUn + LIGHT_MAX_RANGEUn) / vec2(textureSize(lightComputed, 0)));
}

)""