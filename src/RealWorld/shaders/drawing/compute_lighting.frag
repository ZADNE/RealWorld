R""(
#line 3
layout(location = 2) out vec4 computed;

layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(binding = TEX_UNIT_LIGHTS_LIGHT) uniform sampler2D lightSampler;
layout(binding = TEX_UNIT_LIGHTS_TRANSLU) uniform sampler2D transluSampler;

const float DIRECTIONS = 256.0;
const float MIDDLE_RANGE = LIGHT_MAX_RANGEUn * 0.6;
const float PI2 = 6.28318530718;
const float PI_FRACTION = PI2 / DIRECTIONS;

void accumulate(inout vec4 light, vec2 texel){
	vec2 uv = texel / vec2(textureSize(lightSampler, 0));
	light = (light + texture(lightSampler, uv, 0)) * texture(transluSampler, uv, 0).a;
}

void ray(const float maxrange, const float minrange, const float d, inout vec4 light, const vec2 center){
	for (float i = maxrange; i > minrange; --i) {
		accumulate(light, center + vec2(cos(d), sin(d)) * i);
	}
}

void main() {
	vec4 lightSum = vec4(0.0, 0.0, 0.0, 0.0);
	const vec2 center = gl_FragCoord.xy + vec2(0.5);
	//Rays
	for (vec3 dir = vec3(0.0, 1.0, 0.5); dir.x < DIRECTIONS; dir += 2.0) {
		vec3 d = dir * PI_FRACTION;
		vec4 light[2] = vec4[2](vec4(0.0), vec4(0.0));
		ray(LIGHT_MAX_RANGEUn, MIDDLE_RANGE, d[0], light[0], center);
		ray(LIGHT_MAX_RANGEUn, MIDDLE_RANGE, d[1], light[1], center);
		light[0] += light[1];
		ray(MIDDLE_RANGE, 0.0, d[2], light[0], center);
		lightSum += light[0];
	}
	//The center tile itself
	accumulate(lightSum, center);
	computed = vec4(lightSum.rgb, 1.0 - sqrt(lightSum.a));
}

)""