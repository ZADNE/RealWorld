/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_SMOOTHSTEP_GLSL
#define RW_SMOOTHSTEP_GLSL
#include <RealShaders/CppIntegration.glsl>

inline float linstep_x(float x){
    return x;
}

inline float smoothstep_x(float x) {
    return x * x * (3.0f - 2.0f * x);
}

inline float smootherstep_x(float x) {
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

inline vec2 linstep_x_dx(float x){
    return vec2(linstep_x(x), 1.0f);
}

inline vec2 smoothstep_x_dx(float x) {
    return vec2(smoothstep_x(x), -6.0f * (x - 1.0f) * x);
}

inline vec2 smootherstep_x_dx(float x) {
    return vec2(smootherstep_x(x), 30.0f * x * x * (x * (x - 2.0f) + 1.0f));
}

// Remaps range [edge0; edge1] to [0; 1] linearly, clamped outside
//  1>         -----  
//           /        
//          /         
//  0> ----           
//    edge0^   ^edge1
inline float mapToEdges(float edge0, float edge1, float x) {
    return clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
}

inline float linstep_x(float edge0, float edge1, float x){
    return linstep_x(mapToEdges(edge0, edge1, x));
}

inline float smoothstep_x(float edge0, float edge1, float x) {
    return smoothstep(edge0, edge1, x); // Might be more optimal
}

inline float smootherstep_x(float edge0, float edge1, float x) {
    return smootherstep_x(mapToEdges(edge0, edge1, x));
}

// Creates a bump composed of ascending and descending smoothstep functions
//  1>       ^        
//         /   \      
//        /     \     
//  0> --         --  
//   midpoint^   ^midpoint+radius
inline float smoothstepBump(float midpoint, float radius, float x) {
    bool descent = x > midpoint;
    x = descent ? 2.0f * midpoint - x : x;
    return smoothstep(midpoint - radius, midpoint, x);
}

// Creates a bump composed of smoothstep functions with flat top
//  1>        ---------        
//          /           \      
//         /             \     
//  0> ---                 --  
//    t0-r^  ^top0 top1^  ^top1+radius
inline float smoothstepFlatBump(
    float top0, float top1, float radius, float x
) {
    bool descent = x > top1;
    x = descent ? top1 + top0 - x : x;
    return smoothstep(top0 - radius, top0, x);
}

#endif // !RW_SMOOTHSTEP_GLSL