/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_BRANCH_ALLOC_REQ_UB_GLSL
#define RW_BRANCH_ALLOC_REQ_UB_GLSL
#include <RealShaders/CppIntegration.glsl>

#include <RealWorld/constants/World.glsl>

layout (set = 0, binding = k_branchAllocReqBinding, scalar)
restrict uniform BranchAllocReqUB {
    ivec2   targetCh[k_chunkTransferSlots];
    int     branchCount[k_chunkTransferSlots];
    ivec2   worldTexCh;
    int     allocSlotsEnd;
    int     deallocSlotsBegin;
} RE_GLSL_ONLY(u_branchAllocReq);

#endif // !RW_BRANCH_ALLOC_REQ_UB_GLSL