/*!
 *  @author     Dubsky Tomas
 */
#extension GL_EXT_scalar_block_layout : require
layout(push_constant, std430) uniform ShadowDrawingPC {
    mat4    p_viewMat;
    ivec2   p_viewSizeTi;
    vec2    p_botLeftPxModTilePx;
    ivec2   p_readOffsetTi;
};
