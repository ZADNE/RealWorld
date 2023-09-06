/*!
 *  @author     Dubsky Tomas
 */
#ifndef BODIES_SB_GLSL
#define BODIES_SB_GLSL

struct Body {
    ivec2 bottomCenterPx;
    ivec2 sizePx;
    ivec2 velocityPx;
    vec2  rotationRad; /**< y component is unused */
};

layout(set = 0, binding = BodiesSB_BINDING, std430)
restrict buffer BodiesSB {
    uint b_bodiesDispatchX;
    uint b_bodiesDispatchY;
    uint b_bodiesDispatchZ;
    int  b_currentBodyCount;
    int  b_maxBodyCount;
    int  b_bodiesPadding[3];
    Body b_bodies[];
};

#endif // BODIES_SB_GLSL