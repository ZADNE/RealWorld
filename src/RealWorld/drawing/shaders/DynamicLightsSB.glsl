/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_DYNAMIC_LIGHTS_SB_GLSL
#define RW_DYNAMIC_LIGHTS_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

/**
 * @brief Represents a dynamic light that can be added into the world.
 */
struct DynamicLight {
    ivec2 posPx; ///< Position of the center of the light in pixels
    uint col;    ///< RGB = color of the light, A = intensity of the light
    uint padding;
};

#ifdef VULKAN

layout (set = 0, binding = k_dynamicLightsBinding, scalar)
readonly restrict buffer DynamicLightsSB {
    DynamicLight lights[];
} RE_GLSL_ONLY(b_dynamicLights);

#endif

#endif // !RW_DYNAMIC_LIGHTS_SB_GLSL