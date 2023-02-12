/*!
 *  @author     Dubsky Tomas
 */
#extension GL_EXT_scalar_block_layout : require
layout(push_constant, std430) restrict uniform WorldDynamicsPC {
    ivec2   p_globalPosTi;
    uint    p_modifyTarget;
    uint    p_modifyShape;
    uvec2   p_modifySetValue;
    float   p_modifyDiameter;
    uint    p_timeHash;
    //ivec2   p_updateOrder[16];
};
