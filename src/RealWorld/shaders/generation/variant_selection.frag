R""(
#line 3

out uvec4 result;

layout(binding = TEX_UNIT_GEN_TILES0) uniform usampler2D tilesTexture;

const ivec2 OFFSETS[] = {
				{-1, +2},	{+0, +2},	{+1, +2},
	{-2, +1}, 	{-1, +1}, 	{+0, +1}, 	{+1, +1},	{+2, +1},
	{-2, +0}, 	{-1, +0}, 				{+1, +0},	{+2, +0},
	{-2, -1}, 	{-1, -1}, 	{+0, -1}, 	{+1, -1}, 	{+2, -1},
				{-1, -2},	{+0, -2},	{+1, -2}
};
	
void main() {
	ivec2 posTi = ivec2(gl_FragCoord.xy);
	uvec2 block_wall = texelFetch(tilesTexture, posTi, 0).TILE_TYPE;
	
	ivec2 inner = ivec2(0);
	for (int i = 0; i < OFFSETS.length(); i++){
		inner |= ivec2(lessThanEqual(texelFetchOffset(tilesTexture, posTi, 0, OFFSETS[i]).TILE_TYPE, LAST_FLUID.TILE_TYPE));
	}
	
	vec2 variationRange = mix(vec2(12.0), vec2(4.0), bvec2(inner));
	uvec2 variationOffset = mix(uvec2(0), uvec2(12), bvec2(inner));
	
	uvec2 variation = uvec2(hash22(posTi) * variationRange) + variationOffset;
	
	result = uvec4(block_wall.x, variation.x, block_wall.y, variation.y);

}

)""