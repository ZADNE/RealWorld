/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_GENERATE_COLUMN_VALUES_GLSL
#define RW_GENERATE_COLUMN_VALUES_GLSL
#include <RealWorld/generation/external_shaders/smoothstep.glsl>

// x = left val, y = right val, z = 0 to 1 interp. factor between the two
inline vec3 columnValues(float x, float seed){
    float columnX = floor(x);
    float columnFract = fract(x);
    float a = hash12(vec2(columnX, seed));
    float b = hash12(vec2(columnX + 1.0, seed));
    return vec3(a, b, columnFract);
}

inline float linColumnValue_x(float x, float seed){
    vec3 vals = columnValues(x, seed);
    return mix(vals.x, vals.y, linstep_x(vals.z));
}

inline float smoothColumnValue_x(float x, float seed){
    vec3 vals = columnValues(x, seed);
    return mix(vals.x, vals.y, smoothstep_x(vals.z));
}

inline float smootherColumnValue_x(float x, float seed){
    vec3 vals = columnValues(x, seed);
    return mix(vals.x, vals.y, smootherstep_x(vals.z));
}

inline vec2 linColumnValue_x_dx(float x, float seed){
    vec3 vals = columnValues(x, seed);
    vec2 step = linstep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

inline vec2 smoothColumnValue_x_dx(float x, float seed){
    vec3 vals = columnValues(x, seed);
    vec2 step = smoothstep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

inline vec2 smootherColumnValue_x_dx(float x, float seed){
    vec3 vals = columnValues(x, seed);
    vec2 step = smootherstep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

#endif // !RW_GENERATE_COLUMN_VALUES_GLSL