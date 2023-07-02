/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>

#include <RealWorld/save/WorldSave.hpp>

namespace rw {

class TreeSimulator {
public:
    explicit TreeSimulator(const re::PipelineLayout& simulationPL);

    void step(const vk::CommandBuffer& commandBuffer);

    struct Buffers {
        const re::Buffer& rootsBuf;
        const re::Buffer& branchesBuf;
    };

    Buffers adoptSave(const glm::ivec2& worldTexSizeCh);

private:
    struct Root {
        glm::vec2 posPx;
    };

    struct Branch {
        float     lenPx;
        float     rotNaturalRad;
        float     rotCurrentRad;
        glm::uint childrenBeginEnd; // 16-bit indices
    };

    struct BranchesSBHeader {
        glm::uint dispatchX          = 0;
        glm::uint dispatchY          = 1;
        glm::uint dispatchZ          = 1;
        int       currentRootCount   = 0;
        int       maxRootCount       = 0;
        int       currentBranchCount = 0;
        int       maxBranchCount     = 0;
        int       padding[1];
    };

#pragma warning(push)
#pragma warning(disable : 4200)
    struct BranchesSB {
        BranchesSBHeader header;
        Branch           branches[];
    };
#pragma warning(pop)

    /**
     * @brief Size of header is same as 2 branches
     */
    static constexpr int k_branchHeaderSize = sizeof(BranchesSBHeader) /
                                              sizeof(Branch);
    static_assert(k_branchHeaderSize * sizeof(Branch) == sizeof(BranchesSBHeader));

    std::optional<re::Buffer> m_rootsBuf;
    std::optional<re::Buffer> m_branchesBuf;
    re::Pipeline              m_simulateTreesPl;
};

} // namespace rw