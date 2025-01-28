/**
 *  @author     Dubsky Tomas
 */
#ifndef RW_BRANCH_ALLOC_REG_SB_GLSL
#define RW_BRANCH_ALLOC_REG_SB_GLSL
#include <RealShaders/CppIntegration.glsl>

#include <RealWorld/constants/World.glsl>
#include <RealWorld/constants/Vegetation.glsl>

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

#include <RealWorld/simulation/vegetation/shaders/BranchAllocRegSBDebugging.glsl>

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
        return; // No branches allocated for the chunk
    }

    // Free this allocation
    b_branchAllocReg.allocIndexOfTheChunk[chunkIndex] = -1;
    BranchAllocation alloc = b_branchAllocReg.allocations[allocIndex];
    alloc.branchCount = 0;
    b_branchAllocReg.allocations[allocIndex] = BranchAllocation(0, 0, 0, 0, 0);

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

    assertIntegrity();
}

#endif

#endif // !RW_BRANCH_ALLOC_REG_SB_GLSL