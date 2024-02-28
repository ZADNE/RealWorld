/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_ALLOC_REG_SB_GLSL
#define BRANCH_ALLOC_REG_SB_GLSL
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

layout (set = 0, binding = k_branchAllocRegBinding, std430)
restrict coherent buffer BranchAllocRegSB {
    int              allocIndexOfTheChunk[k_maxWorldTexChunkCount];
    BranchAllocation allocations[k_maxBranchAllocCount];
    int              nextAllocIter;
    int              lock;
} b_branchAllocReg;


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
        if (alloc.branchCount == 0) { // If the allocation is empty
            if (alloc.capacity >= toAllocate){ // If the allocation is big enough
                if (alloc.capacity > toAllocate){ // If the allocation is bigger than necessary
                    int nextAllocI = nextAllocIndex(allocIndex);
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
                b_branchAllocReg.allocIndexOfTheChunk[chunkIndex] = allocIndex;
                b_branchAllocReg.nextAllocIter = nextAllocIndex(allocIndex);
                return int(alloc.firstBranch);
            }
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
    if (allocIndex >= 0){// If there is an allocation for the chunk
        // Free this allocation
        b_branchAllocReg.allocIndexOfTheChunk[chunkIndex] = -1;
        BranchAllocation alloc = b_branchAllocReg.allocations[allocIndex];
        alloc.branchCount = 0;

        { // Accumulate capacity of the next allocation
            int nextAllocI = nextAllocIndex(allocIndex);
            BranchAllocation nextAlloc = b_branchAllocReg.allocations[nextAllocI];
            if (nextAlloc.branchCount == 0) {// If the next is empty
                // Accumulate its capacity
                alloc.capacity += nextAlloc.capacity;
                b_branchAllocReg.allocations[nextAllocI].capacity = 0;
            }
        }
        b_branchAllocReg.allocations[allocIndex] = BranchAllocation(0, 0, 0, 0, 0);

        // Sweep and accumulate previous allocations
        int allocSearched = 2;
        BranchAllocation prevAlloc;
        do {
            allocIndex = prevAllocIndex(allocIndex);
            prevAlloc = b_branchAllocReg.allocations[allocIndex];
            if (prevAlloc.branchCount == 0) { // If previous is empty too
                // Accumulate its capacity
                alloc.capacity += prevAlloc.capacity;
                b_branchAllocReg.allocations[allocIndex].capacity = 0;
            } else {
                break; // Sweep finished
            }
        } while (++allocSearched < k_maxBranchAllocCount);

        // Store the accumulated allocation at the swept index
        alloc.firstBranch = prevAlloc.firstBranch + prevAlloc.capacity;
        b_branchAllocReg.allocations[nextAllocIndex(allocIndex)] = alloc;
    }
}

#endif // !BRANCH_ALLOC_REG_SB_GLSL