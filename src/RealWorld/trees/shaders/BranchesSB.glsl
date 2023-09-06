/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCHES_SB_GLSL
#define BRANCHES_SB_GLSL

#include <RealWorld/trees/shaders/Branch.glsl>

struct BranchesSBHeader {
    uint    vertexCount;
    uint    instanceCount;
    uint    firstVertex;
    uint    firstInstance;
    int     maxBranchCount;
    int     padding[3];
};

layout(set = 0, binding = BranchesSBWrite_BINDING, std430)
writeonly restrict buffer BranchesSBWrite {
    BranchesSBHeader    b_branchesHeaderWrite;
    Branch              b_branchesWrite[];
};

layout(set = 0, binding = BranchesSBRead_BINDING, std430)
readonly restrict buffer BranchesSBRead {
    BranchesSBHeader    b_branchesHeaderRead;
    Branch              b_branchesRead[];
};

#endif // BRANCHES_SB_GLSL