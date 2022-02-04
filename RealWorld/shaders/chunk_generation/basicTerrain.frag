R""(
in vec2 pos;

layout(location = 0) out uvec4 result;
layout(location = 1) out uvec4 material;

const float roughnessColumnWidths[4] = float[4](23.0, 13.0, 11.0, 7.0);
const float elevationColumnWidths[4] = float[4](401.0, 197.0, 137.0, 103.0);

vec2 horizon(float x, float seed){
	//Elevation
	float der = 0.0;
	float totalElev = 0.0;
	for (float level = 0.0; level < 4.0; level++){
		vec2 elevation = smootherColumnValue_x_dx(x, elevationColumnWidths[int(level)], seed + level);
		der += elevation.y;
		totalElev += elevation.x * (200.0 - 50.0 * level);
	}
	
	//Roughness
	float totalRough = 0.0;
	for (float level = 0.0; level < 4.0; level++){
		float roughness = linColumnValue_x(x, roughnessColumnWidths[int(level)], seed + level);
		totalRough += roughness * abs(der) * (25.0 - level * 5.0);
	}
	
	float top = totalElev + totalRough;
	float grassLayer = 8.0 * (1.0 - abs(der)) + 12.0;
	return vec2(top, top - grassLayer);
}

float age(vec2 pos, float seed){
	float age = (snoise(pos * 0.001953125, seed) + 1.0) * 0.5;
	return clamp(age * 5.0 - 2.0, 0.0, 1.0);//Amplify and clamp
}

//x = temperature, y = humidity
vec2 biomeClimate(float x, float seed){
	return vec2(
		smoothColumnValue_x(x, 159.0, seed),
		smoothColumnValue_x(x, 159.0, seed + 11.0)
	);
}

float solidity(vec2 pos, float age, float seed){
	vec2 sn = vec2(0.0, 1.0);
	sn.x = hash13(vec3(pos, seed)) * 0.9;
	vec2 p = vec2(pos * (1.0 / 48.0));
	for (float level = 1.0; level <= 1.5; level *= 1.5){
		sn.y = min(abs(snoise(p * level, seed)) * 5.0 / level, sn.y);
	}
	return mix(sn.x, sn.y, age);
}

uvec2 stoneTile(vec2 pos, float age, float seed){
	float dither = hash13(vec3(pos, seed)) * 0.3 - 0.15;
	return STONE_TILES[int(clamp(age + dither, 0.0, 0.9999) * STONE_TILES.length())];
}

uvec2 surfaceTile(vec2 pos, vec2 biomeClimate, float seed){
	vec2 climateDither = hash23(vec3(pos, seed)) * 0.1 - vec2(0.05);
	vec2 climate = clamp(biomeClimate + climateDither, vec2(0.0), vec2(0.9999);
	ivec2 indices = ivec2(vec2(SURFACE_TILES.length(), SURFACE_TILES[0].length()) * climate);
	return SURFACE_TILES[indices.x][indices.y];
}

float horizonProximityFactor(float horizon, float y, float width, float low, float high){
	return clamp(1.0 - (horizon - y) / width, low, high);
}

void main(){
	vec2 p = floor(pos);
	
	float age = age(p, seed);
	vec2 biomeClimate = biomeClimate(p.x, seed);
	float solidity = solidity(p, age, seed);//Decide whether this tile is a solid tile or air
	uvec2 stoneTile = stoneTile(p, age, seed);//Decide which stone tile to use
	uvec2 surfaceTile = surfaceTile(p, biomeClimate, seed);//Decide which surface tile to use
  
	vec2 horizon = horizon(p.x, seed);
	
	bool belowHorizon = (p.y < horizon.r);
	bool belowSoil = (p.y < (horizon.g - hash13(vec3(pos, seed)) * 7.0));
	
	float solidityShifter = belowHorizon ? horizonProximityFactor(horizon.r, p.y, 40.0, 0.0, 1.0 - age * 0.9) : -10.0;
	bool occupied = (solidity + solidityShifter) > 0.5;
	
	material.rb = belowHorizon ? (belowSoil ? stoneTile : surfaceTile) : AIR;	//RB = block & wall type
	material.g = uint((1.0 - age) * 8.01);										//G = number of CA cycles
	material.a = 0;																//A = unused yet
	result = occupied ? material : uvec4(AIR.r, material.gba);
}

)""