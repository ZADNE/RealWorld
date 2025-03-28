/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_WORLD_DRAWING_PC_GLSL
#define RW_WORLD_DRAWING_PC_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (push_constant, scalar)
uniform WorldDrawingPC {
    mat4    minimapViewMat;
    vec2    uvRectSize;
    vec2    uvRectOffset;
    ivec2   botLeftTi;
    ivec2   worldTexMask;
    vec2    minimapOffset;
    vec2    minimapSize;
    vec4    skyColor;
} RE_GLSL_ONLY(p_);

#endif // !RW_WORLD_DRAWING_PC_GLSL