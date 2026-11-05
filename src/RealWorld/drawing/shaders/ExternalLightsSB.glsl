/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_EXTERNAL_LIGHTS_SB_GLSL
#define RW_EXTERNAL_LIGHTS_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

/**
 * @brief Represents a light that can be added into the world.
 */
struct ExternalLight {
    ivec2 posPx; ///< Position of the center of the light in pixels
    float red;   ///< Red light intensity
    float green; ///< Green light intensity
    float blue;  ///< Blue light intensity
    float xlu;   ///< Translucency addition, probably should be zero
};

#ifdef VULKAN

layout (set = 0, binding = k_externalLightsBinding, scalar)
readonly restrict buffer ExternalLightsSB {
    ExternalLight lights[];
} RE_GLSL_ONLY(b_externalLights);

#endif

#endif // !RW_EXTERNAL_LIGHTS_SB_GLSL