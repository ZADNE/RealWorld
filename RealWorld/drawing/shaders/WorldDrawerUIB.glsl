#extension GL_EXT_scalar_block_layout : require
layout(std430, push_constant) uniform WorldDrawerUIB {
    //Updated when view / world texture changes \|/
    mat4 viewMat;
    ivec2 worldTexMask;
    int viewWidthTi;
    uint padding0;
    //Updated every step \|/
    ivec2 analysisOffsetTi;
    vec2 botLeftPxModTilePx;
    ivec2 drawShadowsReadOffsetTi;
    ivec2 botLeftTi;
    ivec2 addLightOffsetPx;
    uint lightCount;
};
