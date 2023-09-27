/*!
 *  @author     Dubsky Tomas
 */
#ifndef GENERATE_COLUMN_VALUES_GLSL
#define GENERATE_COLUMN_VALUES_GLSL

vec3 columnValues(float x, float columnWidth, float seed){
    float ratio = x / columnWidth;
    float columnX = floor(ratio);
    float columnFract = fract(ratio);
    float a = hash12(vec2(columnX, seed));
    float b = hash12(vec2(columnX + 1.0, seed));
    return vec3(a, b, columnFract);
}

float linStep_x(float x){
    return x;
}

float smoothStep_x(float x) {
  return x * x * (3.0 - 2.0 * x);
}

float smootherStep_x(float x) {
  return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

float linColumnValue_x(float x, float columnWidth, float seed){
    vec3 vals = columnValues(x, columnWidth, seed);
    return mix(vals.x, vals.y, linStep_x(vals.z));
}

float smoothColumnValue_x(float x, float columnWidth, float seed){
    vec3 vals = columnValues(x, columnWidth, seed);
    return mix(vals.x, vals.y, smoothStep_x(vals.z));
}

float smootherColumnValue_x(float x, float columnWidth, float seed){
    vec3 vals = columnValues(x, columnWidth, seed);
    return mix(vals.x, vals.y, smootherStep_x(vals.z));
}

vec2 linStep_x_dx(float x){
    return vec2(linStep_x(x), 1);
}

vec2 smoothStep_x_dx(float x) {
  return vec2(smoothStep_x(x), -6.0 * (x - 1.0) * x);
}

vec2 smootherStep_x_dx(float x) {
  return vec2(smootherStep_x(x), 30.0 * x * x * (x * (x - 2.0) + 1.0));
}

vec2 linColumnValue_x_dx(float x, float columnWidth, float seed){
    vec3 vals = columnValues(x, columnWidth, seed);
    vec2 step = linStep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

vec2 smoothColumnValue_x_dx(float x, float columnWidth, float seed){
    vec3 vals = columnValues(x, columnWidth, seed);
    vec2 step = smoothStep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

vec2 smootherColumnValue_x_dx(float x, float columnWidth, float seed){
    vec3 vals = columnValues(x, columnWidth, seed);
    vec2 step = smootherStep_x_dx(vals.z);
    return vec2(mix(vals.x, vals.y, step.x), step.y * (vals.y - vals.x));
}

#endif // !GENERATE_COLUMN_VALUES_GLSL