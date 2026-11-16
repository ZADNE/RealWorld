/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_LIGHT_SWEEP_PC_GLSL
#define RW_LIGHT_SWEEP_PC_GLSL
#include <RealShaders/CppIntegration.glsl>

layout (push_constant, scalar)
uniform LightSweepPC {
    vec2    uvScale;
    vec2    uvOffset; // Offset within max mip
} RE_GLSL_ONLY(p_);

#endif // !RW_LIGHT_SWEEP_PC_GLSL