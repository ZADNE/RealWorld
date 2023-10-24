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

uint branchTexel(uint branchIndex, uvec2 uv){
    return b_branchRaster[branchIndex][2 * uv.y + uv.x];
}

#endif // !BRANCH_RASTER_SB_GLSL