/**
 *  @author     Dubsky Tomas
 */
#ifndef WIND_GLSL
#define WIND_GLSL
#include <RealWorld/generation/external_shaders/snoise.glsl>

float windStrength(vec2 posTi, float timeSec) {
    return snoise(vec2(posTi.x * 0.001, timeSec * 0.1), 0.0) + 
           snoise(vec2(posTi.x * 0.002, timeSec * 0.2), 0.0) * 0.5;
}

#endif // !WIND_GLSL