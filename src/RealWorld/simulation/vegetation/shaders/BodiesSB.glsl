/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_BODIES_SB_GLSL
#define RW_BODIES_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

struct Body {
    ivec2 bottomCenterPx;
    ivec2 sizePx;
    ivec2 velocityPx;
    vec2  rotationRad; ///< y component is unused
};

struct BodiesSBHeader {
    uint bodiesDispatchX;
    uint bodiesDispatchY;
    uint bodiesDispatchZ;
    int  currentBodyCount;
    int  maxBodyCount;
    int  bodiesPadding[3];
};

layout (set = 0, binding = k_bodiesBinding, scalar)
restrict buffer BodiesSB {
    BodiesSBHeader header;
    Body bodies[];
} RE_GLSL_ONLY(b_bodies);

#endif // !RW_BODIES_SB_GLSL