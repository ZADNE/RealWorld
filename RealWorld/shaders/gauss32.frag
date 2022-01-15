R""(
#version 440

in vec2 fragUV;

out vec4 colour;

layout(location = 1) uniform sampler2D baseTexture;

const float directions = 16.0;
const float ranges = 4.0;
const float pi = 6.28318530718;
const float exactWeight = (128.0/256.0);
const float neighborWeight = (19.0/256.0);
const float diagonalWeight = (13.0/256.0);

void main() {
	/*vec2 radius = 1.0 / vec2(textureSize(baseTexture, 0));
	vec4 op = texture(baseTexture, fragUV);
	for (float d = 0.0; d < pi; d += pi / directions) {
		for (float i = 1.0; i <= ranges; ++i) {
			op += texture(baseTexture, fragUV + vec2(cos(d), sin(d)) * radius * i);
		}
	}
	colour = op / (ranges * directions + 1.0);*/

	vec2 radius = 1.0 / vec2(textureSize(baseTexture, 0));
	colour = texture(baseTexture, fragUV) * 0.5;
	colour += texture(baseTexture, fragUV + vec2(radius.x, radius.y)) * diagonalWeight;
	colour += texture(baseTexture, fragUV + vec2(-radius.x, radius.y)) * diagonalWeight;
	colour += texture(baseTexture, fragUV + vec2(-radius.x, -radius.y)) * diagonalWeight;
	colour += texture(baseTexture, fragUV + vec2(radius.x, -radius.y)) * diagonalWeight;

	colour += texture(baseTexture, fragUV + vec2(0.0, radius.y)) * neighborWeight;
	colour += texture(baseTexture, fragUV + vec2(-radius.x, 0.0)) * neighborWeight;
	colour += texture(baseTexture, fragUV + vec2(-0.0, -radius.y)) * neighborWeight;
	colour += texture(baseTexture, fragUV + vec2(radius.x, 0.0)) * neighborWeight;
}
	
)""