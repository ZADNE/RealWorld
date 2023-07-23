/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>

#include <RealWorld/save/WorldSave.hpp>

namespace rw {

class TreeSimulator {
public:
    explicit TreeSimulator(const re::PipelineLayout& simulationPL);

    void step(const vk::CommandBuffer& commandBuffer);

    struct Buffers {
        const re::StepDoubleBuffered<re::Buffer>& branchesBuf;
    };

    Buffers adoptSave(const glm::ivec2& worldTexSizeCh);

private:
    struct Branch {
        struct Angles {
            uint8_t absAngleNorm;     // Absolute
            uint8_t relRestAngleNorm; // Relative to parent
            uint8_t angleVelNorm;     // Absolute
            uint8_t unused{0};
        };
        static_assert(sizeof(Angles) % 4 == 0);

        glm::vec2    absPosPx; // Absolute
        unsigned int parentIndex;
        Angles       angles;
        float        lengthPx;
        float        radiusPx;
        float        density;
        float        stiffness;
    };

    struct BranchesSBHeader {
        glm::uint dispatchX          = 0;
        glm::uint dispatchY          = 1;
        glm::uint dispatchZ          = 1;
        int       currentBranchCount = 0;
        int       maxBranchCount     = 0;
        int       padding[3];
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

    std::optional<re::StepDoubleBuffered<re::Buffer>> m_branchesBuf;
    re::Pipeline                                      m_simulateTreesPl;
};

} // namespace rw
