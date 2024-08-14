/*!
 *  @author     Dubsky Tomas
 */
#ifndef DROPPED_TILES_DRAWING_PC_GLSL
#define DROPPED_TILES_DRAWING_PC_GLSL

#extension GL_EXT_scalar_block_layout : require
layout (push_constant, std430)
uniform DroppedTilesDrawingPC {
    mat4  p_mvpMat;
    float p_blinkState; // 0 == normal, 1 == highlighted
    float p_interpFactor;
};

#endif // !DROPPED_TILES_DRAWING_PC_GLSL