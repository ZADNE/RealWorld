/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_ANALYSIS_PC_GLSL
#define RW_ANALYSIS_PC_GLSL
#include <RealShaders/CppIntegration.glsl>

/**
 * @brief Used by analysis and for adding  of dynamic lights
 */
layout (push_constant, scalar)
uniform AnalysisPC {
    vec4    skyLight; ///< rgb = light intensity, a = unused
    ivec2   worldTexMask;
    ivec2   analysisOffsetTi;
    ivec2   addLightOffsetPx;
    uint    lightCount;
} RE_GLSL_ONLY(p_);

#endif // !RW_ANALYSIS_PC_GLSL