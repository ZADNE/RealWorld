/*!
 *  @author     Dubsky Tomas
 */

struct Body {
    ivec2 bottomCenterPx;
    ivec2 sizePx;
    ivec2 velocityPx;
    vec2  rotationRad; /**< y component is unused */
};

layout(set = 0, binding = BodiesSB_BINDING, std430) restrict buffer BodiesSB {
    uint b_dispatchX;
    uint b_dispatchY;
    uint b_dispatchZ;
    int  b_currentBodyCount;
    int  b_maxBodyCount;
    int  b_padding[3];
    Body b_bodies[];
};
