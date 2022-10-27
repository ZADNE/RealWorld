#version 460
#include <RealWorld/reserved_units/textures.glsl>

layout(location = 0) in vec2 minimapUV;

layout(location = 0) out vec4 outColor;

layout(binding = TEX_UNIT_WORLD_TEXTURE) uniform usampler2D worldTexture;
layout(binding = TEX_UNIT_BLOCK_ATLAS) uniform sampler2D blockAtlas;
layout(binding = TEX_UNIT_WALL_ATLAS) uniform sampler2D wallAtlas;

void main() {
    uvec4 tile = texture(worldTexture, minimapUV);
    vec4 UVs = vec4(tile.yxwz) * (1.0 / vec4(textureSize(blockAtlas, 0), textureSize(wallAtlas, 0)));
    vec4 blockColor = texture(blockAtlas, UVs.xy);
    outColor = mix(texture(wallAtlas, UVs.zw), blockColor, blockColor.a);
}
