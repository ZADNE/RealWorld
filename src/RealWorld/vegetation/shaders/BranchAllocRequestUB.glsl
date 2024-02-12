/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_ALLOC_REQUEST_UB_GLSL
#define BRANCH_ALLOC_REQUEST_UB_GLSL
#include <RealWorld/constants/world.glsl>

#extension GL_EXT_scalar_block_layout : require
layout (set = 0, binding = BranchAllocRequestUB_BINDING, std430)
restrict uniform BranchAllocRequestUB {
    ivec2   targetCh[k_maxParallelTransfers];
    int     branchCount[k_maxParallelTransfers];
    ivec2   worldTexCh;
    int     uploadSlotsEnd;
    int     downloadSlotsBegin;
} u_branchAllocRequest;

#endif // !BRANCH_ALLOC_REQUEST_UB_GLSL