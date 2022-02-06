R""(
in vec2 posTi;

layout(location = 0) out uvec4 result;
layout(location = 1) out uvec4 material;

vec2 horizon(float xMt, float seed){
	//Elevation
	float der = 0.0;
	float totalElev = 0.0;
	for (float level = 1.0; level <= 4.0; level++){
		vec2 elevation = smootherColumnValue_x_dx(xMt, 512.0 / level, seed + level);
		der += elevation.y;
		totalElev += elevation.x * (128.0 / level);
	}
	
	//Roughness
	float totalRough = 0.0;
	/*for (float level = 1.0; level <= 4.0; level++){
		float roughness = linColumnValue_x(xMt, 64.0 / level, seed + level);
		totalRough += roughness * abs(der) * (24.0 / level);
	}*/
	
	float top = totalElev + totalRough;
	float grassLayer = 8.0 * (1.0 - abs(der)) + 12.0;
	return vec2(top, top - grassLayer);
}

float age(vec2 posMt, float seed){
	float age = (snoise(posMt * (1.0 / 2048.0), seed) + 1.0) * 0.5;
	return clamp(age * 5.0 - 2.0, 0.0, 1.0);//Amplify and clamp
}

//x = temperature, y = humidity
vec2 biomeClimate(float x, float seed){
	return vec2(
		linColumnValue_x(x, 1024.0, seed),
		linColumnValue_x(x, 1024.0, seed + 11.0)
	);
}

float solidity(vec2 posMt, float age, float seed){
	vec2 sn = vec2(0.0, 1.0);
	sn.x = hash13(vec3(posMt, seed)) * 0.9;
	vec2 p = vec2(posMt * (1.0 / 192.0));
	for (float level = 1.0; level <= 1.5; level *= 1.5){
		sn.y = min(abs(snoise(p * level, seed)) * 5.0 / level, sn.y);
	}
	return mix(sn.x, sn.y, age);
}

uvec2 stoneTile(vec2 posMt, float age, float seed){
	float dither = hash13(vec3(posMt, seed)) * 0.3 - 0.15;
	return STONE_TILES[int(clamp(age + dither, 0.0, 0.9999) * STONE_TILES.length())];
}

uvec2 surfaceTile(vec2 posMt, vec2 biomeClimate, float seed){
	vec2 climateDither = hash23(vec3(posMt, seed)) * 0.1 - vec2(0.05);
	vec2 climate = clamp(biomeClimate + climateDither, vec2(0.0), vec2(0.9999));
	ivec2 indices = ivec2(vec2(SURFACE_TILES.length(), SURFACE_TILES[0].length()) * climate);
	return SURFACE_TILES[indices.x][indices.y];
}

float horizonProximityFactor(float horizon, float y, float width, float low, float high){
	return clamp(1.0 - (horizon - y) / width, low, high);
}

void main(){
	vec2 pMt = floor(posTi) * tiToMt;
	
	float age = age(pMt, seed);
	vec2 biomeClimate = biomeClimate(pMt.x, seed);
	float solidity = solidity(pMt, age, seed);//Decide whether this tile is a solid tile or air
	uvec2 stoneTile = stoneTile(pMt, age, seed);//Decide which stone tile to use
	uvec2 surfaceTile = surfaceTile(pMt, biomeClimate, seed);//Decide which surface tile to use
  
	vec2 horizon = horizon(pMt.x, seed);
	
	bool belowHorizon = (pMt.y < horizon.r);
	bool belowSoil = (pMt.y < (horizon.g - hash13(vec3(pMt, seed)) * 7.0));
	
	float solidityShifter = belowHorizon ? horizonProximityFactor(horizon.r, pMt.y, 40.0, 0.0, 1.0 - age * 0.9) : -10.0;
	bool occupied = (solidity + solidityShifter) > 0.5;
	
	material.rb = belowHorizon ? (belowSoil ? stoneTile : surfaceTile) : AIR;	//RB = block & wall type
	material.g = 8 + uint((1.0 - age) * 6.01);									//G = number of CA cycles
	material.a = 0;																//A = unused yet
	result = occupied ? material : uvec4(AIR.r, material.gba);
}

)""