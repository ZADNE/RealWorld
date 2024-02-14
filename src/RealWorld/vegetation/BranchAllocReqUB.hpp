/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/constants/world.hpp>

namespace rw {

struct BranchAllocReqUB {
    glm::ivec2 targetCh[k_chunkTransferSlots];
    int        branchCount[k_chunkTransferSlots];
    glm::ivec2 worldTexCh;
    int        uploadSlotsEnd;
    int        downloadSlotsBegin;
};

} // namespace rw
