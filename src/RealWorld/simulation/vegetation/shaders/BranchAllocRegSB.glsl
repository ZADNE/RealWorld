/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_ALLOC_REG_SB_GLSL
#define BRANCH_ALLOC_REG_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

#include <RealWorld/constants/world.glsl>
#include <RealWorld/constants/vegetation.glsl>

// All branches of a chunk belong to one allocation
struct BranchAllocation {
    uint branchCount;
    uint instanceCount;
    uint firstBranch;
    uint firstInstance;

    uint capacity;
};

layout (set = 0, binding = k_branchAllocRegBinding, scalar)
restrict coherent buffer BranchAllocRegSB {
    int              allocIndexOfTheChunk[k_maxWorldTexChunkCount];
    BranchAllocation allocations[k_maxBranchAllocCount];
    int              nextAllocIter;
    int              lock;
} RE_GLSL_ONLY(b_branchAllocReg);

#ifdef VULKAN

void lockAllocRegister(){
    while (atomicCompSwap(b_branchAllocReg.lock, 0, 1) == 1){
        // Spin...
    }
    memoryBarrierBuffer();
}

void unlockAllocRegister(){
    memoryBarrierBuffer();
    atomicExchange(b_branchAllocReg.lock, 0);
}

int nextAllocIndex(uint allocIndex){
    return int((allocIndex + 1) & (k_maxBranchAllocCount - 1));
}

int prevAllocIndex(uint allocIndex){
    return int((allocIndex - 1) & (k_maxBranchAllocCount - 1));
}

bool isLastAllocIndex(uint allocIndex){
    return allocIndex == k_maxBranchAllocCount - 1;
}

uint roundUpToPowerOf2(uint n){
    return 2u << findMSB(n - 1);
}

void printAllocWrite(int index, BranchAllocation alloc) {
    #extension GL_EXT_debug_printf : enable
    debugPrintfEXT(
        "[%i] = {.branchCount = %u, .firstBranch = %u, .capacity = %u}\n",
        index, alloc.branchCount, alloc.firstBranch, alloc.capacity
    );
}

bool g_assertFailed = false;
#define assert(assertion, line)                                 \
    if (!g_assertFailed && !bool(assertion)) {                  \
        debugPrintfEXT("Assertion failed at line: %u\n", line); \
        g_assertFailed = true;                                  \
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
        prevEnd = alloc.firstBranch + alloc.capacity;
        // Capacity
        assert(bool(alloc.branchCount) == allocs[i], __LINE__);
        /*if (g_assertFailed) {
            debugPrintfEXT("allocations[%i].branchCount = %u\n", i, alloc.branchCount);
        }*/
        assert(alloc.branchCount <= alloc.capacity, __LINE__);
        totalCapacity += alloc.capacity;
    }
    assert(totalCapacity == k_maxBranchCount, __LINE__);
}

/**
 * @brief Allocates space for branches
 * @warning The calling thread must have locked the register!
 * @return Index of the first branch; -1 if allocation failed
 */
int allocateBranches(
    uint branchCount, ivec2 chunkCh, ivec2 worldTexSizeCh
) {
    uint toAllocate = roundUpToPowerOf2(max(branchCount, 64));

    int allocSearched = 0;
    int allocIndex = b_branchAllocReg.nextAllocIter;
    do { // While not all allocations have been searched
        BranchAllocation alloc = b_branchAllocReg.allocations[allocIndex];
        if (alloc.branchCount == 0 && alloc.capacity >= toAllocate) { // If the allocation is suitable
            if (alloc.capacity > toAllocate   // If the allocation is bigger than necessary
            &&  !isLastAllocIndex(allocIndex) // and there is a neighbor on the right
            ){
                int nextAllocI = allocIndex + 1;
                BranchAllocation nextAlloc = b_branchAllocReg.allocations[nextAllocI];
                if (nextAlloc.branchCount == 0) { // If the next is empty too
                    // Donate free space to the next allocation
                    uint emptySpace        = alloc.capacity - toAllocate;
                    alloc.capacity         = toAllocate;
                    nextAlloc.firstBranch  = alloc.firstBranch + toAllocate;
                    nextAlloc.capacity    += emptySpace;
                    b_branchAllocReg.allocations[nextAllocI] = nextAlloc;
                }
            }
            // Capture the allocation
            alloc.branchCount = branchCount;
            b_branchAllocReg.allocations[allocIndex] = alloc;
            int chunkIndex = chToIndex(chunkCh, worldTexSizeCh);
            assert(b_branchAllocReg.allocIndexOfTheChunk[chunkIndex] == -1, __LINE__);
            b_branchAllocReg.allocIndexOfTheChunk[chunkIndex] = allocIndex;
            b_branchAllocReg.nextAllocIter = nextAllocIndex(allocIndex);

            assertIntegrity();
            debugPrintfEXT("Allocation at [%v2i]: %u\n\n", chunkCh, uint(!g_assertFailed));
            return int(alloc.firstBranch);
        }
        allocIndex = nextAllocIndex(allocIndex);
    } while (++allocSearched < k_maxBranchAllocCount);

    // No suitable allocation found
    return -1; 
}

/**
 * @brief Frees space allocated for branches of the chunk
 * @warning The calling thread must have locked the register!
 */
void deallocateBranches(
    ivec2 chunkCh, ivec2 worldTexSizeCh
) {
    int chunkIndex = chToIndex(chunkCh, worldTexSizeCh);
    int allocIndex = b_branchAllocReg.allocIndexOfTheChunk[chunkIndex];
    if (allocIndex < 0){
        assert(false, __LINE__);
        debugPrintfEXT("chunkCh [%v2i], worldTexSizeCh [%v2i]\n\n", chunkCh, worldTexSizeCh);
        return; // No branches allocated for the chunk
    }

    // Free this allocation
    b_branchAllocReg.allocIndexOfTheChunk[chunkIndex] = -1;
    BranchAllocation alloc = b_branchAllocReg.allocations[allocIndex];
    alloc.branchCount = 0;
    b_branchAllocReg.allocations[allocIndex] = BranchAllocation(0, 0, 0, 0, 0);
    printAllocWrite(allocIndex, BranchAllocation(0, 0, 0, 0, 0));

    if (!isLastAllocIndex(allocIndex)){ // Accumulate capacity of the next allocation
        int nextAllocI = allocIndex + 1;
        BranchAllocation nextAlloc = b_branchAllocReg.allocations[nextAllocI];
        if (nextAlloc.branchCount == 0) {// If the next is empty
            // Accumulate its capacity
            alloc.capacity += nextAlloc.capacity;
            b_branchAllocReg.allocations[nextAllocI].capacity = 0;
        }
    }

    // Sweep and accumulate allocations to the left
    BranchAllocation prevAlloc;
    while (allocIndex > 0) {
        int prevAllocI = allocIndex - 1;
        prevAlloc = b_branchAllocReg.allocations[prevAllocI];
        if (prevAlloc.branchCount == 0) { // If previous is empty too
            // Accumulate its capacity
            alloc.capacity += prevAlloc.capacity;
            alloc.firstBranch -= prevAlloc.capacity;
            b_branchAllocReg.allocations[prevAllocI].capacity = 0;
            allocIndex = prevAllocI;
        } else {
            break; // Sweep finished
        }
    };

    // Store the accumulated allocation at the swept index
    b_branchAllocReg.allocations[allocIndex] = alloc;
    printAllocWrite(allocIndex, alloc);

    assertIntegrity();
    debugPrintfEXT("Deallocation at [%v2i]: %u\n\n", chunkCh, uint(!g_assertFailed));
}

#endif

#endif // !BRANCH_ALLOC_REG_SB_GLSL