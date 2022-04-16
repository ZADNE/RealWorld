R""(
layout(location = 0) out vec4 lightOut;
layout(location = 1) out vec4 translucencyOut;

layout(binding = TEX_UNIT_WORLD_TEXTURE) uniform usampler2D worldTexture;
layout(binding = TEX_UNIT_BLOCK_LIGHT_ATLAS) uniform sampler2D blockLight;
layout(binding = TEX_UNIT_WALL_LIGHT_ATLAS) uniform sampler2D wallLight;
layout(location = 3) uniform vec2 botLeftTi;

layout(location = 4) uniform vec4 daylight;
layout(location = 5) uniform vec3 translucency;//air, wall, block

const float toUnit = (1.0 / LIGHT_DOWNSAMPLE / LIGHT_DOWNSAMPLE);

void main() {
	float translucencySum = 0.0;
	vec4 lightSum = vec4(0.0);
	
	for (float y = -LIGHT_DOWNSAMPLE; y < 0.0; y++){
		for (float x = -LIGHT_DOWNSAMPLE; x < 0.0; x++){
			uvec4 tile = texelFetch(worldTexture, ivec2(
				mod(botLeftTi + gl_FragCoord.xy * LIGHT_DOWNSAMPLE + vec2(x, y), vec2(textureSize(worldTexture, 0)))), 0);
			bvec2 isAir = equal(tile.TILE_TYPE, AIR.TILE_TYPE);
			if (isAir.x){//If there is air-block
				if (isAir.y){//If there is air-wall
					translucencySum += translucency.x;
					lightSum += daylight;
				} else {//There is not a block but there is a wall
					translucencySum += translucency.y;
					lightSum += texelFetch(wallLight, ivec2(tile.WL_V, tile.WL_T), 0);
				}
			} else {//There is a block
				translucencySum += translucency.z;
				lightSum += texelFetch(blockLight, ivec2(tile.BL_V, tile.BL_T), 0);
			}
		}
	}
	
	lightOut = lightSum * toUnit;
	translucencyOut = vec4(translucencySum) * toUnit;
}

)""