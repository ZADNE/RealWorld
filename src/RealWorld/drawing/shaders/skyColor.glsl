/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SKY_COLOR_GLSL
#define RW_SKY_COLOR_GLSL
#include <RealWorld/constants/Sky.glsl>
#include <RealWorld/generation/external_shaders/smoothstep.glsl>
#include <RealWorld/generation/shaders/tiles/biome.glsl>
#include <RealWorld/generation/shaders/tiles/generateStructure.glsl>

const vec3 k_skyColorsPerBiome[] = vec3[](
    vec3(0.2625, 0.851, 0.952), // Cold
    vec3(0.25411764705, 0.7025490196, 0.90470588235), // Normal
    vec3(0.2895, 0.698, 0.7583) // Hot
);

vec3 calculateBaseBiomeSkyColor(float biomeTemp) {
    // Calculate coords
    biomeTemp = fract(biomeTemp);
    biomeTemp *= 2.0f;
    int ll     = int(biomeTemp);
    float frac = fract(biomeTemp);

    // Gather
    vec3 b00 = k_skyColorsPerBiome[ll];
    vec3 b01 = k_skyColorsPerBiome[ll + 1];

    // Interpolate between biome colors
    return mix(b00, b01, frac);
}

vec3 applyTimeOfDayTint(vec3 sky, float yPx, vec2 elevPx_der, float timeD) {
    float tSd = dToSd(timeD);

    // Dim during night
    float nightDim = mix(
        0.03f,
        1.0f,
        smoothstep(k_sunriseSd - hToD(0.75f), k_sunriseSd + hToD(0.5f), tSd)
    );

    // Orange tint near horizon during sunrise and sunset
    float glowPower = smoothstepBump(k_sunriseSd + hToD(0.5f), hToD(0.5f), tSd);
    float glowTopPx = elevPx_der.x + glowPower * 128.0f + elevPx_der.y * 128.0f;
    float elevFactor = 1.0f - smoothstep(glowTopPx, glowTopPx + 1024.0f, yPx);
    float timeFactor = smoothstepFlatBump(k_sunriseSd - hToD(0.25f), k_sunriseSd + hToD(0.125f), hToD(0.5f), tSd);
    vec3 sunTinted = mix(
        sky * nightDim,
        k_sunsetSkyColor,
        elevFactor * timeFactor * 0.875f
    );
    return sunTinted;
}

vec3 applyOuterSpaceDim(vec3 sky, float yPx) {
    float yScaled = float(yPx - 102400.0f) * 0.00001f;
    float spaceDim = clamp(1.0 - yScaled, 0.0f, 1.0f);
    return sky * spaceDim * spaceDim;
}

vec3 skyColor(
    vec2 viewport01, vec2 pPx, vec2 biomeClimate, float seed, float timeD
) {
    biomeClimate = calcBiomeClimate(pPx.x, seed);

    vec3 base = calculateBaseBiomeSkyColor(biomeClimate.x);

    Biome biome = calcBiomeStructure(biomeClimate);
    vec3 timeTinted = applyTimeOfDayTint(
        base, pPx.y,
        calcElevationWithDer(pPx.x, biome, seed), timeD
    );

    return applyOuterSpaceDim(timeTinted, pPx.y);
}


#endif // !RW_SKY_COLOR_GLSL