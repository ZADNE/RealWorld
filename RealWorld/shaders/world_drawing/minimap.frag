R""(
in vec2 fragUV;

out vec4 fColor;

layout(binding = TEX_UNIT_WORLD_TEXTURE) uniform usampler2D worldTexture;
layout(binding = TEX_UNIT_BLOCK_ATLAS) uniform sampler2D blockTexture;
layout(binding = TEX_UNIT_WALL_ATLAS) uniform sampler2D wallTexture;

void main() {
	uvec4 tile = texture(worldTexture, fragUV);
	vec4 UVs = vec4(tile.yxwz) * (1.0 / vec4(textureSize(blockTexture, 0), textureSize(wallTexture, 0)));
	vec4 blockColor = texture(blockTexture, UVs.xy);
	fColor = mix(texture(wallTexture, UVs.zw), blockColor, blockColor.a);
}
)""