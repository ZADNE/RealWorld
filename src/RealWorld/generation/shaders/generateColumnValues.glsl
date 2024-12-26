/*!
 *  @author     Dubsky Tomas
 */
#ifndef GENERATE_COLUMN_VALUES_GLSL
#define GENERATE_COLUMN_VALUES_GLSL
#include <RealShaders/CppIntegration.glsl>

// x = left val, y = right val, z = 0 to 1 interp. factor between the two
inline vec3 columnValues(float x, float seed){
    float columnX = floor(x);
    float columnFract = fract(x);
    float a = hash12(vec2(columnX, seed));
    float b = hash12(vec2(columnX + 1.0, seed));
    return vec3(a, b, columnFract);
}

inline float linStep_x(float x){
    return x;
}

inline float smoothStep_x(float x) {
  return x * x * (3.0 - 2.0 * x);
}

inline float smootherStep_x(float x) {
  return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

inline float linColumnValue_x(float x, float seed){
    vec3 vals = columnValues(x, seed);
    return mix(vals.x, vals.y, linStep_x(vals.z));
}

inline float smoothColumnValue_x(float x, float seed){
    vec3 vals = columnValues(x, seed);
    return mix(vals.x, vals.y, smoothStep_x(vals.z));
}

inline float smootherColumnValue_x(float x, float seed){
    vec3 vals = columnValues(x, seed);
    return mix(vals.x, vals.y, smootherStep_x(vals.z));
}

inline vec2 linStep_x_dx(float x){
    return vec2(linStep_x(x), 1);
}

inline vec2 smoothStep_x_dx(float x) {
  return vec2(smoothStep_x(x), -6.0 * (x - 1.0) * x);
}

inline vec2 smootherStep_x_dx(float x) {
  return vec2(smootherStep_x(x), 30.0 * x * x * (x * (x - 2.0) + 1.0));
}

inline vec2 linColumnValue_x_dx(float x, float seed){
    vec3 vals = columnValues(x, seed);
    vec2 step = linStep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

inline vec2 smoothColumnValue_x_dx(float x, float seed){
    vec3 vals = columnValues(x, seed);
    vec2 step = smoothStep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

inline vec2 smootherColumnValue_x_dx(float x, float seed){
    vec3 vals = columnValues(x, seed);
    vec2 step = smootherStep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

#endif // !GENERATE_COLUMN_VALUES_GLSL