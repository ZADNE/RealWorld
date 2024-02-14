/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_ALLOC_REQ_UB_GLSL
#define BRANCH_ALLOC_REQ_UB_GLSL
#include <RealWorld/constants/world.glsl>

#extension GL_EXT_scalar_block_layout : require
layout (set = 0, binding = k_branchAllocReqBinding, std430)
restrict uniform BranchAllocReqUB {
    ivec2   targetCh[k_chunkTransferSlots];
    int     branchCount[k_chunkTransferSlots];
    ivec2   worldTexCh;
    int     uploadSlotsEnd;
    int     downloadSlotsBegin;
} u_branchAllocReq;

#endif // !BRANCH_ALLOC_REQ_UB_GLSL