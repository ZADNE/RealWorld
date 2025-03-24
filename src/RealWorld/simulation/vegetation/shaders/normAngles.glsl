/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_NORM_ANGLES_GLSL
#define RW_NORM_ANGLES_GLSL

const float k_pi =  3.14159265359;
const float k_2pi = 6.28318530718;

vec2 toCartesian(float len, float angleNorm) {
    float angle = angleNorm * k_2pi;
    return vec2(len * cos(angle), len * sin(angle));
}

vec2 toCartesian(vec2 size, float angleNorm) {
    float angle = angleNorm * k_2pi;
    float s = sin(angle);
    float c = cos(angle);
    return vec2(dot(size, vec2(-s, c)), dot(size, vec2(c, s)));
}

float angularDifference(float target, float current) {
    float diff = target - current;
    diff += (diff > 0.5f) ? -1.0f : ((diff < -0.5f) ? 1.0f : 0.0f);
    return diff;
}

#endif // !RW_NORM_ANGLES_GLSL