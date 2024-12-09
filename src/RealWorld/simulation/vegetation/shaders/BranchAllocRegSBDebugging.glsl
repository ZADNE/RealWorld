/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_ALLOC_REG_SB_DEBUGGING_GLSL
#define BRANCH_ALLOC_REG_SB_DEBUGGING_GLSL

#ifndef NDEBUG

bool g_assertFailed = false;

#extension GL_EXT_debug_printf : enable
#define assert(assertion, line)                                     \
    if (!g_assertFailed && !bool(assertion)) {                      \
        debugPrintfEXT("Assertion failed at line: %u\n", (line));   \
        g_assertFailed = true;                                      \
    }

void assertIntegrity() {
    // Index integrity
    bool allocs[k_maxBranchAllocCount];
    for (int i = 0; i < k_maxBranchAllocCount; ++i) {
        allocs[i] = false;
    }
    for (int i = 0; i < k_maxWorldTexChunkCount; ++i) {
        int index = b_branchAllocReg.allocIndexOfTheChunk[i];
        if (index >= 0) {
            assert(index < k_maxBranchAllocCount, __LINE__);
            assert(allocs[index] == false, __LINE__);
            allocs[index] = true;
        } else {
            assert(index == -1, __LINE__);
        }
    }
    assert(b_branchAllocReg.nextAllocIter >= 0 && b_branchAllocReg.nextAllocIter < k_maxBranchAllocCount, __LINE__);
    assert(b_branchAllocReg.lock == 1, __LINE__);

    // Allocation integrity
    uint prevEnd       = 0;
    uint totalCapacity = 0;
    for (uint i = 0; i < k_maxBranchAllocCount; i++) {
        const BranchAllocation alloc = b_branchAllocReg.allocations[i];
        // Instance counts
        assert(alloc.instanceCount == 0 || alloc.instanceCount == 1, __LINE__);
        assert(alloc.firstInstance == 0, __LINE__);
        // Continuity
        assert(alloc.firstBranch == prevEnd || alloc.capacity == 0, __LINE__);
        if (g_assertFailed) {
            debugPrintfEXT("prevEnd %u != allocations[%i].firstBranch = %u\n",
                prevEnd, i, alloc.firstBranch
            );
        }
        prevEnd += alloc.capacity;
        // Capacity
        assert(bool(alloc.branchCount) == allocs[i], __LINE__);
        if (g_assertFailed) {
            debugPrintfEXT("allocations[%i].branchCount = %u\n", i, alloc.branchCount);
        }
        assert(alloc.branchCount <= alloc.capacity, __LINE__);
        totalCapacity += alloc.capacity;
    }
    assert(totalCapacity == k_maxBranchCount, __LINE__);
}

#else

#define assert(assertion, line)

#define assertIntegrity()

#endif

#endif // !BRANCH_ALLOC_REG_SB_DEBUGGING_GLSL