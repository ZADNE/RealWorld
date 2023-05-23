/*!
 *  @author     Dubsky Tomas
 */
layout(push_constant, std430) restrict uniform WorldDynamicsPC {
    ivec2   p_globalPosTi;
    uint    p_modifyTarget;
    uint    p_modifyShape;
    uvec2   p_modifySetValue;
    float   p_modifyRadius;
    uint    p_timeHash;
    uint    p_updateOrder;
};
