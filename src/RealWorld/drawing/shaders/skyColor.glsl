/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SKY_COLOR_GLSL
#define RW_SKY_COLOR_GLSL
#include <RealWorld/constants/Sky.glsl>
#include <RealWorld/generation/shaders/tiles/biome.glsl>

const vec3 k_skyColorsPerBiome[] = vec3[](
    vec3(0.2625, 0.851, 0.952), // Cold
    vec3(0.25411764705, 0.7025490196, 0.90470588235), // Normal
    vec3(0.2895, 0.698, 0.7583) // Hot
);

const vec3 k_sunsetSkyColor = vec3(1, 0.624, 0.063);

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

vec3 applyTimeOfDayTint(vec3 sky, float heightTi, float biomeElevTi, float timeD) {
    float t = fract(timeD);
    float timeToSuntouch = min(
        abs(t - k_sunset.r),        // To sunset
        abs(t - (1.0 - k_sunset.r)) // To sunrise
    );
    float timeFactor = 1.0 - timeToSuntouch * 12.0f;

    float biomeElevFactor = clamp((biomeElevTi - heightTi) * 0.01, 0.0f, 0.25f);

    vec3 timeTinted = mix(
        sky,
        k_sunsetSkyColor,
        clamp(timeFactor + biomeElevFactor, 0.0f, 1.0f)
    );
    return timeTinted;
}

vec3 applyOuterSpaceDim(vec3 sky, float heightTi) {
    float heightScaled = float(heightTi - 1024) * 0.001;
    float spaceDim = clamp(1.0 - heightScaled, 0.0, 1.0);
    return sky * spaceDim * spaceDim;
}

vec3 skyColor(vec2 viewport01, float heightTi, vec2 biomeClimate, float timeD) {
    vec3 base = calculateBaseBiomeSkyColor(biomeClimate.x);

    Biome biome = calcBiomeStructure(biomeClimate);
    float biomeElevTi = biome.elevation.x;

    vec3 timeTinted = applyTimeOfDayTint(
        base, heightTi,
        biomeElevTi, timeD
    );

    return applyOuterSpaceDim(timeTinted, heightTi);
}


#endif // !RW_SKY_COLOR_GLSL