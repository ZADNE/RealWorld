/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_UNDERGROUND_MATERIAL_GLSL
#define RW_UNDERGROUND_MATERIAL_GLSL
#include <RealShaders/CppIntegration.glsl>

#include <RealWorld/generation/external_shaders/float_hash.glsl>
#include <RealWorld/generation/external_shaders/snoise.glsl>
#include <RealWorld/generation/shaders/tiles/biome.glsl>

inline uvec2 undergroundMaterial(vec2 posPx, float age, float baseSolidity, float seed){
    float depthFactor = smoothstep(-32768.0f, -8192.0f, posPx.y);
    float lavaFactor = snoise(posPx * (1.0f / 400.0f), -seed) + depthFactor;
    float dither = hash13(vec3(posPx, seed)) * 0.3f - 0.15f;
    uvec2 stoneTile = k_stoneTiles[int(clamp(age + dither, 0.0f, 0.9999f) * 2)];
    return (lavaFactor <= 0.0f && baseSolidity > 0.45f) ? uvec2(k_lavaBl, stoneTile.y) : stoneTile;
}

#endif // !RW_UNDERGROUND_MATERIAL_GLSL