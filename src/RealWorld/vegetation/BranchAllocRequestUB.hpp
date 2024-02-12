/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/world.hpp>

namespace rw {

struct BranchAllocRequestUB {
    glm::ivec2 targetCh[k_maxParallelTransfers];
    int        branchCount[k_maxParallelTransfers];
    glm::ivec2 worldTexCh;
    int        uploadSlotsEnd;
    int        downloadSlotsBegin;
};

} // namespace rw
