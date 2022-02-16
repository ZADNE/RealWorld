R""(
#line 3
in vec2 inChunkPosTi;

layout(location = 0) out uvec4 result;
layout(location = 1) out uvec4 material;

const float PI = 3.14159265359;

float age(vec2 posPx, float seed){
	float age = (snoise(posPx * (1.0 / 8192.0), seed) + 1.0) * 0.5;
	return clamp(age * 5.0 - 2.0, 0.0, 1.0);//Amplify and clamp
}

//x = temperature, y = humidity
vec2 biomeClimate(float x, float seed){
	return vec2(
		smootherColumnValue_x(x, 4096.0, seed),
		smootherColumnValue_x(x, 4096.0, seed + 11.0)
	);
}

vec4 biomeStructure(in vec2 biomeClimate){
	biomeClimate = fract(biomeClimate);
	biomeClimate *= vec2(BIOME_PARAMS.length() - 1, BIOME_PARAMS[0].length() - 1);
	ivec2 ll = ivec2(biomeClimate);
	vec2 frac = fract(biomeClimate);
	vec4 y0 = mix(BIOME_PARAMS[ll.x][ll.y], BIOME_PARAMS[ll.x + 1][ll.y], frac.x);
	vec4 y1 = mix(BIOME_PARAMS[ll.x][ll.y + 1], BIOME_PARAMS[ll.x + 1][ll.y + 1], frac.x);
	return mix(y0, y1, frac.y);
}

vec2 horizon(float xPx, vec4 biomeStructure, float seed){
	//Elevation
	float der = 0.0;
	float totalElev = 0.0;
	vec2 period_amplitude = vec2(2048.0, 1.0);
	for (float level = 0.0; level < 4.0; level++){
		vec2 elevation = smootherColumnValue_x_dx(xPx, period_amplitude.x, seed + level);
		der += elevation.y * period_amplitude.y;
		totalElev += elevation.x * period_amplitude.y * biomeStructure.x;
		period_amplitude *= 0.5;
	}
	der = abs(der) * (1.0 / 1.875);
	
	//Roughness
	period_amplitude = vec2(256.0, biomeStructure.y);
	float totalRough = 0.0;
	for (float level = 0.0; level < 6.0; level++){
		float roughness = linColumnValue_x(xPx, period_amplitude.x, seed + level + 2161.0);
		totalRough += roughness * period_amplitude.y;
		period_amplitude *= 0.5;
	}
	
	float top = totalElev + totalRough * der;
	float surfaceLayer = biomeStructure.z * (1.0 - der) + biomeStructure.w;
	return vec2(top, top - surfaceLayer);
}

float solidity(vec2 posPx, float age, float seed){
	vec2 sn = vec2(0.0, 1.0);
	sn.x = hash13(vec3(posPx, seed)) * 0.9;
	vec2 p = vec2(posPx * (1.0 / 1536.0));
	for (float level = 1.0; level <= 1.5; level *= 1.5){
		sn.y = min(abs(snoise(p * level, seed)) * 6.0 / level, sn.y);
	}
	return mix(sn.x, sn.y, 0.9);
}

uvec2 stoneTile(vec2 posPx, float age, float seed){
	float dither = hash13(vec3(posPx, seed)) * 0.3 - 0.15;
	return STONE_TILES[int(clamp(age + dither, 0.0, 0.9999) * STONE_TILES.length())].TILE_TYPE;
}

uvec2 surfacePxle(vec2 posPx, vec2 biomeClimate, float seed){
	vec2 climateDither = hash23(vec3(posPx, seed)) * 0.1 - vec2(0.05);
	vec2 climate = clamp(biomeClimate + climateDither, vec2(0.0), vec2(0.9999));
	ivec2 indices = ivec2(vec2(SURFACE_TILES.length(), SURFACE_TILES[0].length()) * climate);
	return SURFACE_TILES[indices.x][indices.y].TILE_TYPE;
}

float horizonProximityFactor(float horizon, float y, float width, float low, float high){
	return clamp(1.0 - (horizon - y) / width, low, high);
}

void main(){
	vec2 pPx = (chunkOffsetTi + floor(inChunkPosTi)) * TILEPx;
	
	float age = age(pPx, seed);
	vec2 biomeClimate = biomeClimate(pPx.x, seed);
	vec4 biomeStructure = biomeStructure(biomeClimate);
	float solidity = solidity(pPx, age, seed);//Decide whether this tile is a solid tile or air
	uvec2 stoneTile = stoneTile(pPx, age, seed);//Decide which stone tile to use
	uvec2 surfacePxle = surfacePxle(pPx, biomeClimate, seed);//Decide which surface tile to use
  
	vec2 horizon = horizon(pPx.x, biomeStructure, seed);
	
	bool belowHorizon = (pPx.y < horizon.x);
	bool belowSoil = (pPx.y < (horizon.y + hash13(vec3(pPx, seed)) * biomeStructure.w * 0.25));
	
	float solidityShifter = belowHorizon ? horizonProximityFactor(horizon.x, pPx.y, 400.0, 0.0, 0.2) : -10.0;
	bool occupied = (solidity + solidityShifter) > 0.5;
	
	material.TL_T = belowHorizon ? (belowSoil ? stoneTile : surfacePxle) : AIR.TL_T;//RB = block & wall type
	material.g = 8;// + uint((1.0 - age) * 6.01);									//G = number of CA cycles
	material.a = 0;																	//A = unused yet
	result = occupied ? material : uvec4(AIR.BL_T, material.gba);
}

)""