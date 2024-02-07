/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.hpp>

#include <RealWorld/constants/vegetation.hpp>

namespace rw {

/**
 * @brief All branches of a chunk are inside one allocation
 */
struct BranchAllocation {
    glm::uint vertexCount   = 0;
    glm::uint instanceCount = 1;
    glm::uint firstVertex   = k_maxBranchCount;
    glm::uint firstInstance = 0;
    glm::uint size          = 0;
};

struct BranchAllocRegister {
    std::array<int, k_maxWorldTexChunkCount> allocIndexOfTheChunk{-1};
    std::array<BranchAllocation, k_maxBranchAllocCount> allocations{};
    int                                                 nextAllocIter{};
    int                                                 lock{}; // 0 = unlocked
};

struct BranchSB {
    // Double-buffered params
    glm::vec2 absPosTi[2][k_maxBranchCount];
    float     absAngNorm[2][k_maxBranchCount];

    // Single-buffered params
    glm::uint parentOffset15wallType31[k_maxBranchCount];
    float     relRestAngNorm[k_maxBranchCount];
    float     angVel[k_maxBranchCount];
    float     radiusTi[k_maxBranchCount];
    float     lengthTi[k_maxBranchCount];
    glm::vec2 densityStiffness[k_maxBranchCount];
    uint8_t   raster[k_maxBranchCount][k_branchRasterSpace];

    BranchAllocRegister allocReg;
};

} // namespace rw
