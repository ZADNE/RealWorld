/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_SB_GLSL
#define BRANCH_SB_GLSL
#include <RealShaders/CppIntegration.glsl>
#include <RealWorld/constants/vegetation.glsl>

#ifdef VULKAN
#   extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#endif

layout (set = 0, binding = k_branchBinding, scalar)
restrict buffer BranchSB {
    // Double-buffered params
    vec2    absPosTi[2][k_maxBranchCount];
    float   absAngNorm[2][k_maxBranchCount];

    // Single-buffered params
    uint    parentOffset15wallType31[k_maxBranchCount];// explanation below
    float   relRestAngNorm[k_maxBranchCount];   // rest angle relative to parent
    float   angVel[k_maxBranchCount];           // unorm packed angular velocity
    float   radiusTi[k_maxBranchCount];
    float   lengthTi[k_maxBranchCount];
    vec2    densityStiffness[k_maxBranchCount];
    uint8_t raster[k_maxBranchCount][k_branchRasterByteCount];
} RE_GLSL_ONLY(b_branch);

// parentOffset15wallType31[ 0..15] = negative index offset to parent
// parentOffset15wallType31[16..31] = wall type

#ifdef VULKAN

uint packBranchParentOffsetWallType(uint parentOffset, uint wallType){
    return (parentOffset & 0xffff) | (wallType << 16);
}

uvec2 unpackBranchParentOffsetWallType(uint packed){
    return uvec2(packed & 0xffff, packed >> 16);
}

uvec2 loadBranchParentOffsetWallType(uint branchIndex){
    uint packed = b_branch.parentOffset15wallType31[branchIndex];
    return unpackBranchParentOffsetWallType(packed);
}

void storeBranchParentOffsetWallType(uint branchIndex, uint parentOffset, uint wallType){
    uint packed = packBranchParentOffsetWallType(parentOffset, wallType);
    b_branch.parentOffset15wallType31[branchIndex] = packed;
}

uint branchUvToIndex(ivec2 uv){
    return (((uv.y & 31) * 2) + (uv.x & 1)) * 2;
}

uvec2 loadBranchTexel(uint branchIndex, ivec2 uv){
    uint i = branchUvToIndex(uv);
    return uvec2(b_branch.raster[branchIndex][i], b_branch.raster[branchIndex][i + 1]);
}

void storeBranchTexel(uint branchIndex, ivec2 uv, uvec2 val){
    uint i = branchUvToIndex(uv);
    b_branch.raster[branchIndex][i]     = uint8_t(val.x);
    b_branch.raster[branchIndex][i + 1] = uint8_t(val.y);
}

void storeBranchBasicState(uint branchIndex, ivec2 uv, uint val){
    uint i = branchUvToIndex(uv);
    b_branch.raster[branchIndex][i] = uint8_t(val);
}

const uint k_woodBasicStateNatural = 0;
const uint k_woodBasicStateBurning = 1;
const uint k_woodBasicStateBurnt   = 2;
const uint k_woodBasicStateHallow  = 3;
const uint k_woodBasicStateRemoved = 4;

#endif

#endif // !BRANCH_SB_GLSL