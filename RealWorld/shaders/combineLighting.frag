R""(
#version 460

layout(location = 2) out vec4 colour;

layout(location = 100) uniform sampler2D diaphragm;
layout(location = 101) uniform sampler2D lighting;

//User defined constants
const float DIRECTIONS = 256.0;
const float MAX_RANGE = 30.0;

const float MIDDLE_RANGE = MAX_RANGE * 0.6;
const float PI2 = 6.28318530718;
const float PI_FRACTION = PI2 / DIRECTIONS;

void ray(const float maxrange, const float minrange, const float d, inout vec4 light, const ivec2 center){
	for (float i = maxrange; i > minrange; --i) {
		ivec2 texel = center + ivec2(cos(d) * i, sin(d) * i);
		vec4 dia = texelFetch(diaphragm, texel, 0);
		float d_diff = mod(0.5 + dia.g + d / PI2 + dia.b * 0.5, 1.0);
		if (d_diff <= dia.b) {//If the light is pointing here
			light += texelFetch(lighting, texel, 0);
		}
		light *= dia.r;//Adding darkness
	}
}

void main() {
	vec4 final = vec4(0.0, 0.0, 0.0, 0.0);
	const ivec2 center = ivec2(gl_FragCoord.xy);
	for (vec3 dir = vec3(0.0, 1.0, 0.5); dir.x < DIRECTIONS; dir += 2.0) {
		vec3 d = dir * PI_FRACTION;
		vec4 color[2] = vec4[2](vec4(0.0), vec4(0.0));
		ray(MAX_RANGE, MIDDLE_RANGE, d[0], color[0], center);
		ray(MAX_RANGE, MIDDLE_RANGE, d[1], color[1], center);
		color[0] += color[1];
		ray(MIDDLE_RANGE, 0.0, d[2], color[0], center);
		final += color[0];
	}
	//The tile itself
	vec4 color = texelFetch(lighting, center, 0);
	color *= texelFetch(diaphragm, center, 0).r;
	final += color;
	colour = vec4(final.rgb, 1.0 - sqrt(final.a));
}

)""