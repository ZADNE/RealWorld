// License in file: float_GLSLash-License.txt
#ifndef RW_FLOAT_HASH_GLSL
#define RW_FLOAT_HASH_GLSL
#include <RealShaders/CppIntegration.glsl>

//  1 out, 1 in...
inline float hash11(float p){
    p = fract(p * .1031f);
    p *= p + 33.33f;
    p *= p + p;
    return fract(p);
}
//  1 out, 2 in...
inline float hash12(vec2 p){
    vec3 p3  = fract(vec3(p.xyx) * .1031f);
    p3 += dot(p3, p3.yzx + 33.33f);
    return fract((p3.x + p3.y) * p3.z);
}
//  1 out, 3 in...
inline float hash13(vec3 p3){
    p3  = fract(p3 * .1031f);
    p3 += dot(p3, p3.zyx + 31.32f);
    return fract((p3.x + p3.y) * p3.z);
}
//  2 out, 1 in...
inline vec2 hash21(float p){
    vec3 p3 = fract(vec3(p) * vec3(.1031f, .1030f, .0973f));
    p3 += dot(p3, p3.yzx + 33.33f);
    return fract((p3.xx+p3.yz)*p3.zy);
}
//  2 out, 2 in...
inline vec2 hash22(vec2 p){
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031f, .1030f, .0973f));
    p3 += dot(p3, p3.yzx+33.33f);
    return fract((p3.xx+p3.yz)*p3.zy);
}
//  2 out, 3 in...
inline vec2 hash23(vec3 p3){
    p3 = fract(p3 * vec3(.1031f, .1030f, .0973f));
    p3 += dot(p3, p3.yzx+33.33f);
    return fract((p3.xx+p3.yz)*p3.zy);
}
//  3 out, 1 in...
inline vec3 hash31(float p){
   vec3 p3 = fract(vec3(p) * vec3(.1031f, .1030f, .0973f));
   p3 += dot(p3, p3.yzx+33.33f);
   return fract((p3.xxy+p3.yzz)*p3.zyx); 
}
//  3 out, 2 in...
inline vec3 hash32(vec2 p){
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031f, .1030f, .0973f));
    p3 += dot(p3, p3.yxz+33.33f);
    return fract((p3.xxy+p3.yzz)*p3.zyx);
}
//  3 out, 3 in...
inline vec3 hash33(vec3 p3){
    p3 = fract(p3 * vec3(.1031f, .1030f, .0973f));
    p3 += dot(p3, p3.yxz+33.33f);
    return fract((p3.xxy + p3.yxx)*p3.zyx);
}
// 4 out, 1 in...
inline vec4 hash41(float p){
    vec4 p4 = fract(vec4(p) * vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += dot(p4, p4.wzxy+33.33f);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx); 
}
// 4 out, 2 in...
inline vec4 hash42(vec2 p){
    vec4 p4 = fract(vec4(p.xyxy) * vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += dot(p4, p4.wzxy+33.33f);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}
// 4 out, 3 in...
inline vec4 hash43(vec3 p){
    vec4 p4 = fract(vec4(p.xyzx)  * vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += dot(p4, p4.wzxy+33.33f);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}
// 4 out, 4 in...
inline vec4 hash44(vec4 p4){
    p4 = fract(p4  * vec4(.1031f, .1030f, .0973f, .1099f));
    p4 += dot(p4, p4.wzxy+33.33f);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}

#endif // !RW_FLOAT_HASH_GLSL