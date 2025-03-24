/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_GENERATE_STRUCTURE_GLSL
#define RW_GENERATE_STRUCTURE_GLSL
#include <RealShaders/CppIntegration.glsl>

#include <RealWorld/generation/external_shaders/float_hash.glsl>
#include <RealWorld/generation/external_shaders/snoise.glsl>
#include <RealWorld/generation/shaders/biome.glsl>
#include <RealWorld/constants/Generation.glsl>
#include <RealWorld/generation/shaders/generateColumnValues.glsl>

inline float calcAge(vec2 posPx, float seed){
    float age = snoise(posPx * (1.0f / 8192.0f), seed);
    return clamp(age * 2.0f, -0.5f, +0.5f) + 0.5f; // Oversaturate the age
}

/**
 * @return x = temperature, y = humidity
 */
inline vec2 calcBiomeClimate(float xPx, float seed){
    vec2 res = vec2(0.5f);
    float x = xPx * (1.0f / 8192.0f);
    float amp = 1.0f;

    for (int i = 0; i < 3; ++i){
        res += vec2(linColumnValue_x(x, seed), linColumnValue_x(x, seed + 11.0f)) * amp;
        amp *= 0.5f;
        res -= amp;
        x *= 2.0f;
    }

    return clamp(res, 0.0f, 0.99999f);
}

inline vec2 calcHorizon(float xPx, Biome biome, float seed){
    // Elevation
    float der = 0.0f;
    float totalElev = biome.elevation.x;
    vec2 period_amplitude = vec2(2048.0f, 1.0f);
    for (float level = 0.0f; level < 4.0f; level++){
        vec2 elevation = smootherColumnValue_x_dx(xPx / period_amplitude.x, seed + level);
        der += elevation.y * period_amplitude.y;
        totalElev += elevation.x * period_amplitude.y * biome.elevation.y;
        period_amplitude *= 0.5f;
    }
    der = abs(der) * (1.0f / 1.875f);

    // Roughness
    period_amplitude = vec2(256.0f, 1.0f);
    float totalRough = 0.0f;
    for (float level = 0.0f; level < 6.0f; level++){
        totalRough += linColumnValue_x(xPx / period_amplitude.x, seed + level + 2161.0f) * period_amplitude.y;
        period_amplitude *= 0.5f;
    }

    float top = totalElev + totalRough * (biome.roughness.x + biome.roughness.y * der);
    float surfaceLayer = biome.surfaceWidth.x + biome.surfaceWidth.y * (1.0f - der);
    return vec2(top, top - surfaceLayer);
}


inline float calcSolidity(vec2 posPx, float age, float seed){
    const vec2 k_caveWidthFactor = vec2(0.2f, 0.8f);

    vec2 p = vec2(posPx * (1.0f / 1536.0f));
    vec2 solidity = vec2(10.0f, 0.0f);
    for (float level = 1.0f; level <= 1.5f; level += 0.5f){
        solidity.x = min(abs(snoise(p * level, seed)) / level, solidity.x);
    }

    for (float level = 1.0f; level <= 4.0f; level *= 2.0f){
        solidity.y += abs(snoise(p * level, seed)) / level;
    }
    solidity /= k_caveWidthFactor;

    vec2 solidity_weight = mix(vec2(solidity.x, 0.85f), vec2(solidity.y, 0.92f), age);

    return mix(hash13(vec3(posPx, seed)), solidity_weight[0], solidity_weight[1]);
}

inline uvec2 calcStoneTile(vec2 posPx, float age, float baseSolidity, float seed){
    float depthFactor = smoothstep(-32768.0f, -8192.0f, posPx.y);
    float lavaFactor = snoise(posPx * (1.0f / 400.0f), -seed) + depthFactor;
    float dither = hash13(vec3(posPx, seed)) * 0.3f - 0.15f;
    uvec2 stoneTile = k_stoneTiles[int(clamp(age + dither, 0.0f, 0.9999f) * 2)];
    return (lavaFactor <= 0.0f && baseSolidity > 0.45f) ? uvec2(k_lavaBl, stoneTile.y) : stoneTile;
}

inline uvec2 calcSurfaceTile(vec2 posPx, float seed){
    float posDither = (hash13(vec3(posPx, seed)) - 0.5f) * 2048.0f;
    vec2 climate = calcBiomeClimate(posPx.x + posDither, seed);
    ivec2 indices = ivec2(vec2(k_biomesMatrixSize) * climate);
    return k_biomes[indices.x][indices.y].tiles;
}

inline float caclHorizonProximityFactor(float horizon, float y, float width, float low, float high){
    return clamp(1.0f - (horizon - y) / width, low, high);
}

struct GeneratedTile{
    uvec4 tile;
    uvec4 material;
};

inline GeneratedTile calcBasicTerrain(in vec2 pPx, in float seed){
    float age = calcAge(pPx, seed);
    float solidity = calcSolidity(pPx, age, seed);
    uvec2 stoneTile = calcStoneTile(pPx, age, solidity, seed); // Decides which underground tile to use 
    uvec2 surfaceTile = calcSurfaceTile(pPx, seed); // Decide which surface tile to use

    vec2 biomeClimate = calcBiomeClimate(pPx.x, seed);
    Biome biome = calcBiomeStructure(biomeClimate);
    vec2 horizon = calcHorizon(pPx.x, biome, seed);

    bool belowHorizon = (pPx.y < horizon.x);
    bool belowSoil = (pPx.y < (horizon.y + hash13(vec3(pPx, seed)) *
                     (biome.surfaceWidth.x + biome.surfaceWidth.y) * 0.25f));

    float solidityShifter =
        belowHorizon
            ? caclHorizonProximityFactor(horizon.x, pPx.y, 400.0f, 0.0f, 0.2f)
            : -10.0f;
    bool occupied = (solidity + solidityShifter) > 0.5f;

    GeneratedTile rval;
    rval.material.rb = belowHorizon ? (belowSoil ? stoneTile : surfaceTile) : k_air;// RB = block & wall type
    rval.material.ga = uvec2(255, 255);
    rval.tile = occupied ? rval.material : uvec4(k_airBl, rval.material.gba);
    return rval;
}

#endif // !RW_GENERATE_STRUCTURE_GLSL