R""(
#line 3

out vec4 fColor;

layout(binding = TEX_UNIT_WORLD_TEXTURE) uniform usampler2D worldTexture;
layout(location = 3) uniform vec2 realPosPx;
layout(binding = TEX_UNIT_BLOCK_ATLAS) uniform sampler2D blockTexture;
layout(binding = TEX_UNIT_WALL_ATLAS) uniform sampler2D wallTexture;
uniform vec4 perTileUVIncrement;

const vec2 POS[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};
const vec2 UV[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

void main() {
	vec2 posTi = POS[gl_VertexID] + vec2(gl_InstanceID % viewWidthTi, gl_InstanceID / viewWidthTi);
	
	vec2 globalWorldPosTi = posTi + floor(realPosPx / TILEPx) - UV[gl_VertexID];

	vec2 worldTexSize = vec2(textureSize(worldTexture, 0));
	
	vec2 localWorldPosTi = mod(globalWorldPosTi, worldTexSize);
	
	vec4 tile = vec4(texelFetch(worldTexture, ivec2(localWorldPosTi), 0));
	
	//Clip if both block and wall are air
	float clip = (isAirBlock(uint(tile.BL_T)) && isAirWall(uint(tile.WL_T))) ? -1.0 : 1.0;
	
	gl_Position = viewsizePxMat * vec4(posTi * TILEPx - mod(realPosPx, TILEPx), 0.0, clip);
	
	vec4 UVs = tile.yxwz * (1.0 / vec4(textureSize(blockTexture, 0), textureSize(wallTexture, 0)));
	
	vec4 blockColor = texture(blockTexture, UVs.xy);
	fColor = mix(texture(wallTexture, UVs.zw), blockColor, blockColor.a);
}

)""