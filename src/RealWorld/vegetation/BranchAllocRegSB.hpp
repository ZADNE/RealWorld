/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <algorithm>
#include <array>

#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/constants/world.hpp>

namespace rw {

// All branches of a chunk belong to one allocation
struct BranchAllocation {
    glm::uint branchCount{};
    glm::uint instanceCount{};
    glm::uint firstBranch{};
    glm::uint firstInstance{};

    glm::uint capacity{};
};

struct BranchAllocRegSB {
    BranchAllocRegSB() {
        std::fill(allocIndexOfTheChunk.begin(), allocIndexOfTheChunk.end(), -1);
    }

    std::array<int, k_maxWorldTexChunkCount>            allocIndexOfTheChunk{};
    std::array<BranchAllocation, k_maxBranchAllocCount> allocations{};
    int                                                 nextAllocIter{};
    int                                                 lock{}; // 0 = unlocked
};

} // namespace rw
