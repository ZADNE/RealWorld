/*!
 *  @author     Dubsky Tomas
 */
 
struct Body {
    vec2 bottomCenterPx;
    vec2 sizePx;
    vec2 velocityPx;
    vec2 rotationRad; /**< y component is unused */
};

layout(set = 0, binding = BodiesSB_BINDING, std430) restrict buffer BodiesSB {
    int b_currentBodyCount;
    int b_maxBodyCount;
    int b_padding[6];
    Body bodies;
};
