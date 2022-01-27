R""(
#version 460

in vec4 fragColour;
in float lightDir;
in float lightCone;
in vec2 interpolation;

layout(location = 0) out vec4 diaphragm;
layout(location = 1) out vec4 light;

layout(location = 100) uniform sampler2D dia;

void main() {
	vec2 pointCoord = (gl_PointCoord - 0.25) * 2.0;
	light = fragColour * abs(1.0 - pointCoord.x - interpolation.x) * abs(1.0 - pointCoord.y - interpolation.y);
	diaphragm = texelFetch(dia, ivec2(gl_FragCoord.xy), 0);
	diaphragm.g = lightDir;
	diaphragm.b = lightCone;
}

)""