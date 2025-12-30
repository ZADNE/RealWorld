/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SKY_LIGHT_GLSL
#define RW_SKY_LIGHT_GLSL
#include <RealWorld/constants/Sky.glsl>
#include <RealWorld/generation/external_shaders/smoothstep.glsl>

vec3 skyLight(float timeD) {
    float tSd = dToSd(timeD);

    float power = smootherstep_x(k_sunriseSd, k_fullSunlightSd, tSd);

    // Orange tint during sunrise and sunset
    vec3 tint = mix(
        vec3(1.0f),
        k_sunsetSkyColor,
        smoothstepFlatBump(k_sunriseSd - hToD(0.25f), k_sunriseSd + hToD(0.125f), hToD(0.5f), tSd) * 0.75f
    );

    return vec3(power) * tint + k_backgroundLight;
}

#endif // !RW_SKY_LIGHT_GLSL