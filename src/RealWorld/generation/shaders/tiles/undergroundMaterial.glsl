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

inline uvec4 rockyStone(vec2 posPx, float seed){
    vec2 distHash = voronoiBorderNoise(posPx * (1.0f / 256.0f), seed);
    bool cold = distHash.y > 0.5;
    return uvec4(
        cold ? k_coldStoneBl : k_stoneBl,
        distHash.x * k_1OVerSqrt2 * 255,
        cold ? k_coldStoneWl : k_stoneWl,
        distHash.x * k_1OVerSqrt2 * 255
    );
}

inline uvec4 undergroundMaterial(vec2 posPx, float age, float baseSolidity, float seed){
    return rockyStone(posPx, seed);
    /*float depthFactor = smoothstep(-32768.0f, -8192.0f, posPx.y);
    float lavaFactor = snoise(posPx * (1.0f / 400.0f), -seed) + depthFactor;
    float dither = hash13(vec3(posPx, seed)) * 0.3f - 0.15f;
    uvec2 stoneTile = k_stoneTiles[int(clamp(age + dither, 0.0f, 0.9999f) * 2)];
    return (lavaFactor <= 0.0f && baseSolidity > 0.45f) ? uvec2(k_lavaBl, stoneTile.y) : stoneTile;*/
}

#endif // !RW_UNDERGROUND_MATERIAL_GLSL