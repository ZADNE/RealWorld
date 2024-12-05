/*!
 *  @author     Dubsky Tomas
 */
#ifndef BODIES_SB_GLSL
#define BODIES_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

struct Body {
    ivec2 bottomCenterPx;
    ivec2 sizePx;
    ivec2 velocityPx;
    vec2  rotationRad; ///< y component is unused
};

layout (set = 0, binding = k_bodiesBinding, scalar)
restrict buffer BodiesSB {
    uint bodiesDispatchX;
    uint bodiesDispatchY;
    uint bodiesDispatchZ;
    int  currentBodyCount;
    int  maxBodyCount;
    int  bodiesPadding[3];
    Body bodies[];
} RE_GLSL_ONLY(b_bodies);

#endif // !BODIES_SB_GLSL