/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SKY_LIGHT_GLSL
#define RW_SKY_LIGHT_GLSL

/// Light that is always present
const vec3 k_backgroundLight = vec3(0.008f, 0.020f, 0.073f) / 1024.0f;

#if 1

vec3 smootherStep(vec3 x) {
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

vec3 smootherStep(vec3 edge0, vec3 edge1, vec3 x) {
    return smootherStep(clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f));
}

const vec3 k_descentStart = vec3(13.5f, 13.5f, 13.5f) / 24.0f;
const vec3 k_fromNoonToDescent = k_descentStart - 0.5f;
const vec3 k_sunset = vec3(18.1f, 18.0f, 18.0f) / 24.0f;
const vec3 k_fromNoonToSunset = k_sunset - 0.5f;

vec3 skyLight(float timeD) {
    vec3 t = vec3(fract(timeD));
    vec3 fromNoon = vec3(abs(0.5f - t));
    
    bvec3 isNight = greaterThan(fromNoon, k_fromNoonToSunset);
    bvec3 isNoon = lessThan(fromNoon, k_fromNoonToDescent);
    
    vec3 smoothVal = smootherStep(k_fromNoonToSunset, k_fromNoonToDescent, fromNoon);
    
    vec3 light = mix(mix(smoothVal, vec3(1.0f), isNoon), vec3(0.0f), isNight);
    
    return light + k_backgroundLight;
}

#else

#define PI 3.141592653589793f

const vec3 k_skyLightRange[] = vec3[](
    vec3(0.25f, 0.247f, 0.247f), // AM
    vec3(0.22f, 0.25f, 0.25f)  // PM
);

vec3 skyRangeToFreqScale(vec3 skyRange){
    return 1.0f / (0.5f - skyRange);
}

const vec3 k_skyFreqScale[] = vec3[](
    1.0f / (0.5f - k_skyLightRange[0]), // AM
    1.0f / (0.5f - k_skyLightRange[1])  // PM
);

vec3 skyLight(float timeD) {
    float t = fract(timeD);
    vec3 skyFreqScale = k_skyFreqScale[int(t > 0.5f)];
    vec3 range = k_skyLightRange[int(t > 0.5f)];
     
    vec3 cosine = cos((t - 0.5f) * skyFreqScale * PI * 0.5f);
    vec3 cosineSq = cosine * cosine;
    vec3 light = cosineSq * cosineSq;
    vec3 inRange = vec3(greaterThan(vec3(t), range))
                   * vec3(lessThan(vec3(t), 1.0f - range));
    return inRange * light + k_backgroundLight;
}

#endif

#endif // !RW_SKY_LIGHT_GLSL