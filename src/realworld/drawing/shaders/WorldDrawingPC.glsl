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
    vec2    botLeftPx;
    ivec2   worldTexMask;
    vec2    minimapOffset;
    vec2    minimapSize;
    vec2    biomeClimate; ///< Climate in the center of the view
    float   seed;
    float   timeD;
} RE_GLSL_ONLY(p_);

#endif // !RW_WORLD_DRAWING_PC_GLSL