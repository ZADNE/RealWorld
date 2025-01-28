/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_WIND_GLSL
#define RW_WIND_GLSL
#include <RealWorld/generation/external_shaders/snoise.glsl>

float windStrength(vec2 posTi, float timeSec) {
    return snoise(vec2(posTi.x * 0.001, timeSec * 0.1), 0.0) + 
           snoise(vec2(posTi.x * 0.002, timeSec * 0.2), 0.0) * 0.5;
}

#endif // !RW_WIND_GLSL