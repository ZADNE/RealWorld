/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_VECTOR_SB_GLSL
#define BRANCH_VECTOR_SB_GLSL

#include <RealWorld/vegetation/shaders/Branch.glsl>

struct BranchVectorSBHeader {
    uint    vertexCount;
    uint    instanceCount;
    uint    firstVertex;
    uint    firstInstance;
    int     maxBranchCount;
    int     padding[3];
};

layout (set = 0, binding = BranchVectorSBWrite_BINDING, std430)
writeonly restrict buffer BranchVectorSBWrite {
    BranchVectorSBHeader    b_branchesHeaderWrite;
    Branch                  b_branchesWrite[];
};

layout (set = 0, binding = BranchVectorSBRead_BINDING, std430)
readonly restrict buffer BranchVectorSBRead {
    BranchVectorSBHeader    b_branchesHeaderRead;
    Branch                  b_branchesRead[];
};

#endif // !BRANCH_VECTOR_SB_GLSL