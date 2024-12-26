// License in file: snoise-License.txt
#ifndef SNOISE_GLSL
#define SNOISE_GLSL
#include <RealShaders/CppIntegration.glsl>

inline vec2 hash(vec2 p){
    p = vec2(dot(p,vec2(127.1f, 311.7f)), dot(p,vec2(269.5f, 183.3f)));
    return -1.0f + 2.0f*fract(sin(p) * 43758.5453123f);
}

inline float snoise(in vec2 p, float seed){
    const float K1 = 0.366025404f; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865f; // (3-sqrt(3))/6;

    vec2  i = floor(p + (p.x + p.y) * K1);
    vec2  a = p - i + (i.x + i.y) * K2;
    float m = step(a.y, a.x); 
    vec2  o = vec2(m, 1.0f - m);
    vec2  b = a - o + K2;
    vec2  c = a - 1.0f + 2.0f * K2;
    vec3  h = max(0.5f - vec3(dot(a, a), dot(b, b), dot(c, c) ), 0.0f);
    vec3  n = h*h*h*h*vec3(
        dot(a, hash(i + seed)),
        dot(b, hash(i + o + seed)),
        dot(c, hash(i + 1.0f + seed)));
    return dot(n, vec3(70.0f));
}

#endif // !SNOISE_GLSL