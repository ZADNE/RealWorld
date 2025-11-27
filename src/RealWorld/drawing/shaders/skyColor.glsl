/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SKY_COLOR_GLSL
#define RW_SKY_COLOR_GLSL

const vec3 k_skyColors[] = vec3[](
    vec3(0.2625, 0.851, 0.952), // Cold
    vec3(0.25411764705, 0.7025490196, 0.90470588235), // Normal
    vec3(0.2895, 0.698, 0.7583) // Hot
);

vec3 skyColor(float biomeTemp) {
    // Calculate coords
    biomeTemp = fract(biomeTemp);
    biomeTemp *= 2.0f;
    int ll     = int(biomeTemp);
    float frac = fract(biomeTemp);

    // Gather
    vec3 b00 = k_skyColors[ll];
    vec3 b01 = k_skyColors[ll + 1];

    // Interpolate
    return mix(b00, b01, frac);
}


#endif // !RW_SKY_COLOR_GLSL