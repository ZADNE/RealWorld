/*!
 *  @author     Dubsky Tomas
 */
#ifndef BRANCH_SB_WITH_ALLOCATION_GLSL
#define BRANCH_SB_WITH_ALLOCATION_GLSL
#include <RealWorld/vegetation/shaders/BranchSB.glsl>

void lockAllocRegister(){
    while (atomicCompSwap(b_branch.allocReg.lock, 0, 1) == 1){
        // Spin...
    }
}

void unlockAllocRegister(){
    atomicExchange(b_branch.allocReg.lock, 0);
}

int nextAllocIndex(uint allocIndex){
    return int((allocIndex + 1) & (k_maxBranchAllocCount - 1));
}

uint roundUpToPowerOf2(uint n){
    return 2u << findMSB(n - 1);
}

/**
 * @brief Allocates space for branches
 * @return Index of the first branch; -1 if allocation failed
 */
int allocateBranches(
    uint toAllocate, ivec2 chunkCh, ivec2 worldTexSizeCh
) {
    toAllocate = roundUpToPowerOf2(max(toAllocate, 128));

    lockAllocRegister();
    int allocSearched = 0;
    int allocIndex = b_branch.allocReg.nextAllocIter;
    do { // While not all allocations have been searched
        BranchAllocation alloc = b_branch.allocReg.allocations[allocIndex];
        if (alloc.vertexCount == 0) { // If the allocation is empty
            if (alloc.size >= toAllocate){ // If the allocation is big enough
                if (alloc.size > toAllocate){ // If the allocation is bigger than necessary
                    int nextAllocI = nextAllocIndex(allocIndex);
                    BranchAllocation nextAlloc = b_branch.allocReg.allocations[nextAllocI];
                    if (nextAlloc.vertexCount == 0) { // If the next is empty too
                        // Donate free space to the next allocation
                        uint diff = alloc.size - toAllocate;
                        alloc.size            -= diff;
                        nextAlloc.firstVertex -= diff;
                        nextAlloc.size        += diff;
                        b_branch.allocReg.allocations[nextAllocI] = nextAlloc;
                        }
                }
                // Capture the allocation
                alloc.vertexCount = toAllocate;
                b_branch.allocReg.allocations[allocIndex] = alloc;
                int chunkIndex = chToIndex(chunkCh, worldTexSizeCh);
                b_branch.allocReg.allocIndexOfTheChunk[chunkIndex] = allocIndex;
                b_branch.allocReg.nextAllocIter = nextAllocIndex(allocIndex);
                unlockAllocRegister();
                return int(alloc.firstVertex);
            }
        }
        allocIndex = nextAllocIndex(allocIndex);
    } while (++allocSearched < k_maxBranchAllocCount);

    // No suitable allocation found
    unlockAllocRegister();
    return -1; 
}

#endif // !BRANCH_SB_WITH_ALLOCATION_GLSL