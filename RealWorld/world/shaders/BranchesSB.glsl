/*!
 *  @author     Dubsky Tomas
 */

struct Branch {
    vec2    absPosTi; // Absolute
    uint    parentIndex;
    uint    angles;
    float   radiusTi;
    float   lengthTi;
    float   density;
    float   stiffness;
};

struct BranchesSBHeader {
    uint    branchCount;
    uint    instanceCount;
    uint    firstBranch;
    uint    firstInstance;
    int     maxBranchCount;
    int     padding[3];
};

layout(set = 0, binding = BranchesSBWrite_BINDING, std430)
writeonly restrict buffer BranchesSBWrite {
    BranchesSBHeader    header;
    Branch              branches[];
} b_branchesWrite;

layout(set = 0, binding = BranchesSBRead_BINDING, std430)
readonly restrict buffer BranchesSBRead {
    BranchesSBHeader    header;
    Branch              branches[];
} b_branchesRead;
