/*!
 *  @author     Dubsky Tomas
 */
#ifndef ANALYSIS_PC_GLSL
#define ANALYSIS_PC_GLSL

#extension GL_EXT_scalar_block_layout : require
layout(push_constant, std430)
uniform AnalysisPC {
    ivec2   p_worldTexMask;
    ivec2   p_analysisOffsetTi;
    ivec2   p_addLightOffsetPx;
    uint    p_lightCount;
};

#endif // !ANALYSIS_PC_GLSL