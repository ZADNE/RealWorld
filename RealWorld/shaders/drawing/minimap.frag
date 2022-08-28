R""(
in vec2 vUV;

out vec4 fColor;

layout(binding = TEX_UNIT_WORLD_TEXTURE) uniform usampler2D worldTexture;
layout(binding = TEX_UNIT_BLOCK_ATLAS) uniform sampler2D blockAtlas;
layout(binding = TEX_UNIT_WALL_ATLAS) uniform sampler2D wallAtlas;

void main() {
    uvec4 tile = texture(worldTexture, vUV);
    vec4 UVs = vec4(tile.yxwz) * (1.0 / vec4(textureSize(blockAtlas, 0), textureSize(wallAtlas, 0)));
    vec4 blockColor = texture(blockAtlas, UVs.xy);
    fColor = mix(texture(wallAtlas, UVs.zw), blockColor, blockColor.a);
}
)""