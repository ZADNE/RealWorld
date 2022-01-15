R""(
#version 460

in vec2 pos;

layout(location = 0) out uvec4 result;
layout(location = 1) out uvec4 material;

layout(std140) uniform ChunkUniforms {
	mat4 chunkGenMatrix;
	vec2 chunkOffsetBc;
	vec2 chunkDims;
	vec2 chunkBorders;
	int seed;
};

const uvec2 AIR = uvec2(0, 0);
const uvec2 GRASS[1] = uvec2[1](
			uvec2(3, 3)
);
const uvec2 STONE[2] = uvec2[2](
			uvec2(1, 1),
			uvec2(18, 5)
);

float hash12(vec2 p){
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float hash13(vec3 p3){
	p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

vec2 hash23(vec3 p3){
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

vec2 hash(vec2 p){
	p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float snoise(in vec2 p, float seed){
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

	vec2  i = floor(p + (p.x + p.y) * K1);
    vec2  a = p - i + (i.x + i.y) * K2;
    float m = step(a.y, a.x); 
    vec2  o = vec2(m, 1.0 - m);
    vec2  b = a - o + K2;
	vec2  c = a - 1.0 + 2.0 * K2;
    vec3  h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c) ), 0.0 );
	vec3  n = h*h*h*h*vec3(
        dot(a, hash(i + seed)),
        dot(b, hash(i + o + seed)),
        dot(c, hash(i + 1.0 + seed)));
    return dot(n, vec3(70.0));
}

vec3 columnValues(float x, float columnSize, float seed){
	float ratio = x / columnSize;
    float columnX = floor(ratio);
    float columnFract = fract(ratio);
    float a = hash12(vec2(columnX, seed));
    float b = hash12(vec2(columnX + 1.0, seed));
    return vec3(a, b, columnFract);
}

vec2 lin_step(float x){
    return vec2(x, 1);
}

vec2 smooth_step(float x) {
  return vec2(x * x * (3.0 - 2.0 * x), -6.0 * (x - 1.0) * x);
}

vec2 smoother_step(float x) {
  return vec2(
      x * x * x * (x * (x * 6.0 - 15.0) + 10.0),
      30.0 * x * x * (x * (x - 2.0) + 1.0));
}

vec2 linColumnValue(float x, float columnSize, float seed){
    vec3 vals = columnValues(x, columnSize, seed);
    vec2 step = lin_step(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

vec2 smoothColumnValue(float x, float columnSize, float seed){
    vec3 vals = columnValues(x, columnSize, seed);
    vec2 step = smooth_step(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

vec2 smootherColumnValue(float x, float columnSize, float seed){
    vec3 vals = columnValues(x, columnSize, seed);
    vec2 step = smoother_step(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

const float roughnessColumnWidths[4] = float[4](23.0, 13.0, 11.0, 7.0);
const float elevationColumnWidths[4] = float[4](401.0, 197.0, 137.0, 103.0);

vec2 horizon(float x, float seed){
    //Elevation
    float der = 0.0;
    float totalElev = 0.0;
    for (float level = 0.0; level < 4.0; level++){
        vec2 elevation = smootherColumnValue(x, elevationColumnWidths[int(level)], seed + level);
        der += elevation.g;
        totalElev += elevation.r * (200.0 - 50.0 * level);
    }
    
    //Roughness
    float totalRough = 0.0;
    for (float level = 0.0; level < 4.0; level++){
        vec2 roughness = linColumnValue(x, roughnessColumnWidths[int(level)], seed + level);
        totalRough += roughness.r * abs(der) * (25.0 - level * 5.0);
    }
    
	float top = totalElev + totalRough;
    float grassLayer = 8.0 * (1.0 - abs(der)) + 12.0;
    return vec2(top, top - grassLayer);
}

float age(vec2 pos, float seed){
    return (snoise(pos * 0.001953125, seed) + 1.0) * 0.5;
}

float solidness(vec2 pos, float age, float seed){
    vec2 sn = vec2(0.0, 1.0);
    sn.x = hash13(vec3(pos, seed));
    vec2 p = vec2(pos * (1.0 / 64.0));
    for (float level = 1.0; level <= 1.0; level *= 2.0){
        sn.y = min(abs(snoise(p, seed)), sn.y);
    }
    sn.y *= 3.0;
    
    return mix(sn.x, sn.y, age);
}

uvec2 stoneType(vec2 pos, float age, float seed){
	float shift = hash13(vec3(pos, seed)) * 0.3 - 0.15;
    return ((age + shift) < 0.5) ? STONE[0] : STONE[1];
}

void main(){
	vec2 p = floor(pos);
	float s = float(seed);
	
	float age = age(p, s);
    age = clamp(age * 5.0 - 2.0, 0.0, 1.0);//Amplify and clip age
    float sn = solidness(p, age, s);
	uvec2 stone = stoneType(p, age, s);//Decide which stone to use
  
	vec2 horizon = horizon(p.x, s);
	
	bool belowHorizon = (p.y < horizon.r);
    bool belowSoil = (p.y < (horizon.g - hash13(vec3(pos, seed)) * 7.0));
	
    float snShifter = belowHorizon ? clamp(1.0 - (horizon.r - p.y) / 80.0, 0.0, 0.1) : -10.0;
    bool occupied = (sn + snShifter) > 0.5;
    
    material.rb = belowHorizon ? (belowSoil ? stone : GRASS[0]) : AIR;		//RB = block & wall type
	material.g = 4 + uint(clamp((1.0 - age) * 5.0 - 2.0, 0.0, 1.0) * 4.01);	//G = number of CA cycles
	material.a = 0;															//A = unused yet
	result = occupied ? material : uvec4(AIR.r, material.gba);
}

)""