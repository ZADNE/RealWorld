R""(
#version 440

in vec4 fragColour;
in float lightDir;
in float lightCone;

layout(location = 0) out vec4 diaphragm;
layout(location = 1) out vec4 light;

layout(location = 100) uniform sampler2D dia;
//layout(location = 101) uniform sampler2D lighting;

void main() {
	//float prevStr = texelFetch(lighting, ivec2(gl_FragCoord.xy), 0).a;
	vec4 d = texelFetch(dia, ivec2(gl_FragCoord.xy), 0);
	diaphragm = vec4(d.r, lightDir, lightCone, fragColour.a / d.a);
	light = fragColour;
}

)""