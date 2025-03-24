/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <bitset>
#include <cassert>
#include <memory>

#include <glm/vec2.hpp>

#include <RealWorld/constants/Vegetation.hpp>
#include <RealWorld/simulation/vegetation/shaders/BranchAllocRegSB_glsl.hpp>

namespace rw {

/**
 * @brief Allocates BranchAllocRegSB initializes it
 * @details It is allocated on heap because it would take too much space on stack.
 */
inline std::unique_ptr<glsl::BranchAllocRegSB> createBranchAllocRegSB() {
    auto ptr = std::make_unique<glsl::BranchAllocRegSB>();
    for (auto& index : ptr->allocIndexOfTheChunk) { // NOLINT(*-pointer-decay)
        index = -1;
    }
    ptr->allocations[0].capacity = k_maxBranchCount;
    return ptr;
}

/**
 * @brief Checks integrity of allocation register
 * @warning The function is slow. It is intended for debugging purposes only!
 */
inline void assertIntegrity(const glsl::BranchAllocRegSB& r) {
    // Index integrity
    std::bitset<k_maxBranchAllocCount> allocs;
    for (auto index : r.allocIndexOfTheChunk) {
        if (index >= 0) {
            assert(index < k_maxBranchAllocCount);
            assert(allocs[index] == false);
            allocs[index] = true;
        } else {
            assert(index == -1);
        }
    }
    assert(r.nextAllocIter >= 0 && r.nextAllocIter < k_maxBranchAllocCount);
    assert(r.lock == 0);

    // Allocation integrity
    glm::uint prevEnd       = 0;
    glm::uint totalCapacity = 0;
    for (size_t i = 0; i < k_maxBranchAllocCount; i++) {
        const auto& alloc = r.allocations[i];
        // Instance counts
        assert(alloc.instanceCount == 0 || alloc.instanceCount == 1);
        assert(alloc.firstInstance == 0);
        // Continuity
        assert(alloc.firstBranch == prevEnd || alloc.capacity == 0);
        prevEnd = alloc.firstBranch + alloc.capacity;
        // Capacity
        assert(static_cast<bool>(alloc.branchCount) == allocs[i]);
        assert(alloc.branchCount <= alloc.capacity);
        totalCapacity += alloc.capacity;
    }
    assert(totalCapacity == k_maxBranchCount);
}

} // namespace rw
