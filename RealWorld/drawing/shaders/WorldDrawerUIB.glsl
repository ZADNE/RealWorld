layout(std140, binding = 0) uniform WorldDrawerUIB {
    mat4 viewMat;
    ivec2 worldTexMask;
    int viewWidthTi;
};
