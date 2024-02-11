/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <RealWorld/constants/vegetation.hpp>

namespace rw {

/**
 * @brief All branches of a chunk are inside one allocation
 */
struct BranchAllocation {
    glm::uint activeBranchCount = 0;
    glm::uint instanceCount     = 1;
    glm::uint firstBranch       = k_maxBranchCount;
    glm::uint firstInstance     = 0;

    glm::uint branchCount = 0;
    glm::uint capacity    = 0;
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

struct BranchSerialized {
    static_assert(
        offsetof(BranchSB, allocReg) == 7602176,
        "Layout of branch members probably changed - fix me"
    );
    template<typename T>
    using NoRef = std::remove_reference_t<T>;
    using B     = BranchSB;

    NoRef<decltype(*B::absPosTi[0])>   absPosTi;
    NoRef<decltype(*B::absAngNorm[0])> absAngNorm;

    NoRef<decltype(*B::parentOffset15wallType31)> parentOffset15wallType31;
    NoRef<decltype(*B::relRestAngNorm)>           relRestAngNorm;
    NoRef<decltype(*B::angVel)>                   angVel;
    NoRef<decltype(*B::radiusTi)>                 radiusTi;
    NoRef<decltype(*B::lengthTi)>                 lengthTi;
    NoRef<decltype(*B::densityStiffness)>         densityStiffness;
    NoRef<decltype(*B::raster)>                   raster;

    static constexpr size_t memberCount() {
        static_assert(
            offsetof(BranchSB, allocReg) == 7602176,
            "Also fix the count of members here"
        );
        return 9;
    }
};

} // namespace rw
