/*!
 *  @author     Dubsky Tomas
 */
#ifndef WORLD_DRAWING_PC_GLSL
#define WORLD_DRAWING_PC_GLSL

#extension GL_EXT_scalar_block_layout : require
layout (push_constant, std430)
uniform WorldDrawingPC {
    mat4    p_minimapViewMat;
    vec2    p_uvRectSize;
    vec2    p_uvRectOffset;
    ivec2   p_botLeftTi;
    ivec2   p_worldTexMask;
    vec2    p_minimapOffset;
    vec2    p_minimapSize;
};

#endif // !WORLD_DRAWING_PC_GLSL