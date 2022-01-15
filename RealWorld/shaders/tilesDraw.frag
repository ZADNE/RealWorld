R""(
#version 440

in vec2 blockUV;
in vec2 wallUV;

out vec4 colour;

uniform sampler2D blockTexture;
uniform sampler2D wallTexture;

void main() {
	vec4 block = texture(blockTexture, blockUV);
	colour = mix(texture(wallTexture, wallUV), block, block.a);
}

)""