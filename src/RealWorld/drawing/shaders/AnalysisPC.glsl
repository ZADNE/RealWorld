/**
 *  @author     Dubsky Tomas
 */
#ifndef ANALYSIS_PC_GLSL
#define ANALYSIS_PC_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (push_constant, scalar)
uniform AnalysisPC {
    vec4    skyLight;
    ivec2   worldTexMask;
    ivec2   analysisOffsetTi;
    ivec2   addLightOffsetPx;
    uint    lightCount;
} RE_GLSL_ONLY(p_);

#endif // !ANALYSIS_PC_GLSL