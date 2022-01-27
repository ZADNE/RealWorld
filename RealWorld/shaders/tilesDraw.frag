R""(
#version 460

in vec4 block_wall_UV;

out vec4 colour;

uniform sampler2D blockTexture;
uniform sampler2D wallTexture;

void main() {
	vec4 block = texture(blockTexture, block_wall_UV.xy);
	colour = mix(texture(wallTexture, block_wall_UV.zw), block, block.a);
}

)""