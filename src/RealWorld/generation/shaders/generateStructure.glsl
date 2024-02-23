/*!
 *  @author     Dubsky Tomas
 */
#ifndef GENERATE_STRUCTURE_GLSL
#define GENERATE_STRUCTURE_GLSL

#include <RealWorld/generation/external_shaders/float_hash.glsl>
#include <RealWorld/generation/external_shaders/snoise.glsl>
#include <RealWorld/generation/shaders/biome.glsl>
#include <RealWorld/constants/generation.glsl>
#include <RealWorld/generation/shaders/generateColumnValues.glsl>
#include <RealWorld/generation/shaders/GenerationPC.glsl>

float age(vec2 posPx, float seed){
    float age = snoise(posPx * (1.0 / 8192.0), seed);
    return clamp(age * 2.0, -0.5, +0.5) + 0.5; // Oversaturate the age
}

/**
 * @return x = temperature, y = humidity
 */
vec2 biomeClimate(float xPx, float seed){
    vec2 res = vec2(0.0);
    float x = xPx * (1.0 / 8192.0);
    float amp = 0.5;

    for (int i = 0; i < 3; ++i){
        res +=
            vec2(linColumnValue_x(x, seed), linColumnValue_x(x, seed + 11.0)) * amp;
        x *= 2.0;
        amp *= 0.5;
    }

    return res;
}

vec2 horizon(float xPx, Biome biome, float seed){
    // Elevation
    float der = 0.0;
    float totalElev = biome.elevation.x;
    vec2 period_amplitude = vec2(2048.0, 1.0);
    for (float level = 0.0; level < 4.0; level++){
        vec2 elevation = smootherColumnValue_x_dx(xPx / period_amplitude.x, seed + level);
        der += elevation.y * period_amplitude.y;
        totalElev += elevation.x * period_amplitude.y * biome.elevation.y;
        period_amplitude *= 0.5;
    }
    der = abs(der) * (1.0 / 1.875);

    // Roughness
    period_amplitude = vec2(256.0, 1.0);
    float totalRough = 0.0;
    for (float level = 0.0; level < 6.0; level++){
        totalRough += linColumnValue_x(xPx / period_amplitude.x, seed + level + 2161.0) * period_amplitude.y;
        period_amplitude *= 0.5;
    }

    float top = totalElev + totalRough * (biome.roughness.x + biome.roughness.y * der);
    float surfaceLayer = biome.surfaceWidth.x + biome.surfaceWidth.y * (1.0 - der);
    return vec2(top, top - surfaceLayer);
}


float solidity(vec2 posPx, float age, float seed){
    const vec2 CAVE_WIDTH_FACTOR = vec2(0.2, 0.8);

    vec2 p = vec2(posPx * (1.0 / 1536.0));
    vec2 solidity = vec2(10.0, 0.0);
    for (float level = 1.0; level <= 1.5; level += 0.5){
        solidity.x = min(abs(snoise(p * level, seed)) / level, solidity.x);
    }

    for (float level = 1.0; level <= 4.0; level *= 2.0){
        solidity.y += abs(snoise(p * level, seed)) / level;
    }
    solidity /= CAVE_WIDTH_FACTOR;

    vec2 solidity_weight = mix(vec2(solidity.x, 0.85), vec2(solidity.y, 0.92), age);

    return mix(hash13(vec3(posPx, seed)), solidity_weight[0], solidity_weight[1]);
}

uvec2 stoneTile(vec2 posPx, float age, float baseSolidity, float seed){
    float depthFactor = smoothstep(-32768.0, -8192.0, posPx.y);
    float lavaFactor = snoise(posPx * (1.0 / 400.0), -seed) + depthFactor;
    float dither = hash13(vec3(posPx, seed)) * 0.3 - 0.15;
    uvec2 stoneTile = k_stoneTiles[int(clamp(age + dither, 0.0, 0.9999) * k_stoneTiles.length())];
    return (lavaFactor <= 0.0 && baseSolidity > 0.45) ? uvec2(k_lavaBl, stoneTile.y) : stoneTile;
}

uvec2 surfaceTile(vec2 posPx, float seed){
    float posDither = (hash13(vec3(posPx, seed)) - 0.5) * 2048.0;
    vec2 climate = biomeClimate(posPx.x + posDither, seed);
    ivec2 indices = ivec2(vec2(k_biomes.length(), k_biomes[0].length()) * climate);
    return k_biomes[indices.x][indices.y].tiles;
}

float horizonProximityFactor(float horizon, float y, float width, float low, float high){
    return clamp(1.0 - (horizon - y) / width, low, high);
}

void basicTerrain(in vec2 pPx, out uvec4 tile, out uvec4 material){
    float age = age(pPx, p_seed);
    float solidity = solidity(pPx, age, p_seed);
    uvec2 stoneTile = stoneTile(pPx, age, solidity, p_seed); // Decides which underground tile to use 
    uvec2 surfaceTile = surfaceTile(pPx, p_seed); // Decide which surface tile to use

    vec2 biomeClimate = biomeClimate(pPx.x, p_seed);
    Biome biome = biomeStructure(biomeClimate);
    vec2 horizon = horizon(pPx.x, biome, p_seed);

    bool belowHorizon = (pPx.y < horizon.x);
    bool belowSoil = (pPx.y < (horizon.y + hash13(vec3(pPx, p_seed)) * (biome.surfaceWidth.x + biome.surfaceWidth.y) * 0.25));

    float solidityShifter = belowHorizon ? horizonProximityFactor(horizon.x, pPx.y, 400.0, 0.0, 0.2) : -10.0;
    bool occupied = (solidity + solidityShifter) > 0.5;

    material.rb = belowHorizon ? (belowSoil ? stoneTile : surfaceTile) : k_air;//RB = block & wall type
    material.ga = uvec2(255, 255);

    tile = occupied ? material : uvec4(k_airBl, material.gba);
}

#endif // !GENERATE_STRUCTURE_GLSL