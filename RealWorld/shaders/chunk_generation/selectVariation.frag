R""(
#line 3

out uvec4 result;

layout(binding = TEX_UNIT_CHUNK_TILES1) uniform usampler2D worldTexture;

const ivec2 offsets[] = ivec2[](
					ivec2(-1, -2),	ivec2(+0, -2),	ivec2(+1, -2),
	ivec2(-2, -1), 	ivec2(-1, -1), 	ivec2(+0, -1), 	ivec2(+1, -1), 	ivec2(+2, -1),
	ivec2(-2, +0), 	ivec2(-1, +0), 				 	ivec2(+1, +0),	ivec2(+2, +0),
	ivec2(-2, +1), 	ivec2(-1, +1), 	ivec2(+0, +1), 	ivec2(+1, +1),	ivec2(+2, +1),
					ivec2(-1, +2),	ivec2(+0, +2),	ivec2(+1, +2)
);
	
void main() {
	ivec2 posTi = ivec2(gl_FragCoord.xy);
	uvec2 block_wall = texelFetch(worldTexture, posTi, 0).rb;
	
	ivec2 inner = ivec2(0);
	for (int i = 0; i < offsets.length(); i++){
		inner |= ivec2(equal(texelFetchOffset(worldTexture, posTi, 0, offsets[i]).rb, AIR));
	}
	
	vec2 variationRange = mix(vec2(12.0), vec2(4.0), bvec2(inner));
	uvec2 variationOffset = mix(uvec2(0), uvec2(12), bvec2(inner));
	
	uvec2 variation = uvec2(hash22(posTi) * variationRange) + variationOffset;
	
	result = uvec4(block_wall.x, variation.x, block_wall.y, variation.y);

}

)""