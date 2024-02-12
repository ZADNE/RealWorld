/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_SB_GLSL
#define BRANCH_SB_GLSL
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#include <RealWorld/constants/world.glsl>
#include <RealWorld/constants/vegetation.glsl>

// All allocations within the list belong to one chunk
struct BranchAllocation {
    uint activeBranchCount;
    uint instanceCount;
    uint firstBranch;
    uint firstInstance;

    uint branchCount;
    uint capacity;
};

struct BranchAllocRegister {
    int              allocIndexOfTheChunk[k_maxWorldTexChunkCount];
    BranchAllocation allocations[k_maxBranchAllocCount];
    int              nextAllocIter;
    int              lock;
};

layout (set = 0, binding = BranchSB_BINDING, std430)
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
    uint8_t raster[k_maxBranchCount][k_branchRasterSpace];

    BranchAllocRegister allocReg;
} b_branch;

// parentOffset15wallType31[ 0..15] = negative index offset to parent
// parentOffset15wallType31[16..31] = wall type

uvec2 loadBranchParentOffsetWallType(uint branchIndex){
    uint packed = b_branch.parentOffset15wallType31[branchIndex];
    return uvec2(packed & 0xffff, packed >> 16);
}

void storeBranchParentOffsetWallType(uint branchIndex, uint parentOffset, uint wallType){
    uint packed = (parentOffset & 0xffff) | (wallType << 16);
    b_branch.parentOffset15wallType31[branchIndex] = packed;
}

uint loadBranchTexel(uint branchIndex, ivec2 uv){
    uint i = ((uv.y & 31) * 2) + (uv.x & 1);
    return b_branch.raster[branchIndex][i];
}

void storeBranchTexel(uint branchIndex, ivec2 uv, uint val){
    uint i = ((uv.y & 31) * 2) + (uv.x & 1);
    b_branch.raster[branchIndex][i] = uint8_t(val);
}

#endif // !BRANCH_SB_GLSL