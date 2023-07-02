/*!
 *  @author     Dubsky Tomas
 */

struct Branch {
    float lenPx;
    float naturalRotRad;
    float currentRotRad;
    uint  childrenBeginEnd; // 16-bit indices
};

layout(set = 0, binding = BranchesSB_BINDING, std430) restrict buffer BranchesSB {
    uint   b_branchesDispatchX;
    uint   b_branchesDispatchY;
    uint   b_branchesDispatchZ;
    int    b_currentRootCount;
    int    b_maxRootCount;
    int    b_currentBranchCount;
    int    b_maxBranchCount;
    int    b_branchesPadding[3];
    Branch b_branches[];
};
