/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_UNDERGROUND_MATERIAL_GLSL
#define RW_UNDERGROUND_MATERIAL_GLSL
#include <RealShaders/CppIntegration.glsl>

#include <RealWorld/generation/external_shaders/float_hash.glsl>
#include <RealWorld/generation/external_shaders/snoise.glsl>
#include <RealWorld/generation/shaders/tiles/biome.glsl>
#include <RealWorld/generation/shaders/tiles/voronoiNoise.glsl>

const float k_1OVerSqrt2 = 0.70710678118f; // 1 / sqrt(2)

inline uvec4 rockyStoneMaterial(vec2 posPx, float seed){
    // Determine size of rocks
    /*float rockScale = 1.0f;
    float octaveFrequency = 1.0f;
    float octaveAmplitude = 0.25f;
    for (int i = 0; i < 3; ++i){
        vec2 pos = posPx * (1.0f / 512.0f) * octaveFrequency;
        float hash = hash12(voronoiBorderNoise(pos, seed + 1337.0f).xy);
        rockScale += (hash - 0.5f) * octaveAmplitude;
        octaveAmplitude *= 0.5f;
        octaveFrequency *= 2.0f;
    }*/
    vec3 featPosDist = voronoiBorderNoise(posPx * (1.0f / 32.0f), seed);
    bool cold = hash12(featPosDist.xy) > 0.5f;
    return uvec4(
        cold ? k_coldStoneBl : k_stoneBl,
        featPosDist.z * k_1OVerSqrt2 * 255,
        cold ? k_coldStoneWl : k_stoneWl,
        featPosDist.z * k_1OVerSqrt2 * 255
    );
}

inline uvec4 ditherStoneMaterial(vec2 posPx, float age, float seed){
    float ageDither = hash13(vec3(posPx, seed)) * 0.3f - 0.15f;
    uvec2 stoneTile = k_stoneTiles[int(clamp(age + ageDither, 0.0f, 0.9999f) * 2)];
    uvec2 tileVars = uvec2(hash23(vec3(posPx, seed)) * 255.0f);
    return uvec4(stoneTile.x, tileVars.x, stoneTile.y, tileVars.y);
}

inline uvec4 undergroundMaterial(vec2 posPx, float age, float baseSolidity, float seed){
    float stoneDither = hash12(posPx) * 0.1f - 0.05f;
    float stoneStructure = snoise(posPx * (1.0f / 8192.0f), seed + 363.0f) + stoneDither; 
    uvec4 stoneTile = (stoneStructure > 0.0f)
                      ? rockyStoneMaterial(posPx, seed)
                      : ditherStoneMaterial(posPx, age, seed);
    float depthFactor = smoothstep(-32768.0f, -8192.0f, posPx.y);
    float lavaFactor = snoise(posPx * (1.0f / 400.0f), -seed) + depthFactor;
    return (lavaFactor <= 0.0f && baseSolidity > 0.45f)
           ? uvec4(k_lavaBl, 0, stoneTile.zw)
           : stoneTile;
}

#endif // !RW_UNDERGROUND_MATERIAL_GLSL