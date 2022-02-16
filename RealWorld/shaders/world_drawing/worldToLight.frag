R""(
layout(location = 0) out vec4 diaphragm;
layout(location = 1) out vec4 light;

layout(binding = TEX_UNIT_WORLD_TEXTURE) uniform usampler2D worldTexture;
layout(location = 3) uniform vec2 botLeftTi;

layout(location = 4) uniform vec4 daylight;
layout(location = 5) uniform vec3 diaphragms;//air, wall, block

void main() {
	float diaSum = 0.0;
	vec4 lightSum = vec4(0.0);
	
	for (float y = -LIGHT_DOWNSAMPLE; y < 0.0; y++){
		for (float x = -LIGHT_DOWNSAMPLE; x < 0.0; x++){
			uvec4 tile = texelFetch(worldTexture, ivec2(
				mod(botLeftTi + gl_FragCoord.xy * LIGHT_DOWNSAMPLE + vec2(x, y), vec2(textureSize(worldTexture, 0)))), 0);
			bvec2 isAir = equal(tile.rb, AIR.xz);
			if (isAir.x){//If there is air-block
				if (isAir.y){//If there is air-wall
					diaSum += diaphragms.x;
					lightSum += daylight;
				} else {//There is not a block but there is a wall
					diaSum += diaphragms.y;
				}
			} else {//There is a block
				diaSum += diaphragms.z;
			}
		}
	}
	
	diaphragm = vec4(diaSum * (1.0 / LIGHT_DOWNSAMPLE / LIGHT_DOWNSAMPLE), 0.0, 1.0, 0.0);
	light = lightSum * (1.0 / LIGHT_DOWNSAMPLE / LIGHT_DOWNSAMPLE);
}

)""