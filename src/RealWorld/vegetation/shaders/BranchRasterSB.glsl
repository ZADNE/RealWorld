/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_RASTER_SB_GLSL
#define BRANCH_RASTER_SB_GLSL
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require

layout (set = 0, binding = BranchRasterSB_BINDING, std430)
restrict buffer BranchRasterSB {
    uint8_t b_branchRaster[][64];
};

uint branchTexel(uint branchIndex, ivec2 uv){
    uint i = ((uv.y & 31) * 2) + (uv.x & 1);
    return b_branchRaster[branchIndex][i];
}

void setBranchTexel(uint branchIndex, ivec2 uv, uint val){
    uint i = ((uv.y & 31) * 2) + (uv.x & 1);
    b_branchRaster[branchIndex][i] = uint8_t(val);
}

#endif // !BRANCH_RASTER_SB_GLSL