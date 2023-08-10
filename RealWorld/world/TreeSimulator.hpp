﻿/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/graphics/buffers/Buffer.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/output_control/Framebuffer.hpp>
#include <RealEngine/graphics/output_control/RenderPass.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>

namespace rw {

class TreeSimulator {
public:
    TreeSimulator();

    void step(const vk::CommandBuffer& commandBuffer);

    struct Buffers {
        const re::StepDoubleBuffered<re::Buffer>& branchesBuf;
    };

    Buffers adoptSave(const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh);

private:
    struct Branch {
        struct Angles {
            uint8_t absAngleNorm;     // Absolute
            uint8_t relRestAngleNorm; // Relative to parent
            uint8_t angleVelNorm;     // Absolute
            uint8_t unused{0};
        };
        static_assert(sizeof(Angles) % 4 == 0);

        glm::vec2    absPosTi;
        unsigned int parentIndex;
        Angles       angles;
        float        radiusTi;
        float        lengthTi;
        float        density;
        float        stiffness;
    };

    struct BranchesSBHeader {
        uint32_t vertexCount    = 0;
        uint32_t instanceCount  = 1;
        uint32_t firstVertex    = 0;
        uint32_t firstInstance  = 0;
        int      maxBranchCount = 0;
        int      padding[3];
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

    struct TreeDynamicsPC {
        glm::mat4 mvpMat;
        glm::vec2 worldTexSizeTi;
        float     timeSec = static_cast<float>(time(nullptr) & 0xFFFF);
    };
    TreeDynamicsPC m_treeDynamicsPC;

    re::PipelineLayout m_pipelineLayout;
    re::RenderPass     m_renderPass;
    re::Pipeline       m_simulateAndRasterizeBranchesPl{
              {.pipelineLayout     = *m_pipelineLayout,
               .topology           = vk::PrimitiveTopology::ePatchList,
               .patchControlPoints = 1,
               .enableBlend        = false,
               .renderPass         = *m_renderPass},
              {.vert = simulAndRasterizeTrees_vert,
               .tesc = simulAndRasterizeTrees_tesc,
               .tese = simulAndRasterizeTrees_tese,
               .frag = simulAndRasterizeTrees_frag}};
    re::StepDoubleBuffered<re::DescriptorSet> m_descriptorSets{
        re::DescriptorSet{m_pipelineLayout.descriptorSetLayout(0)},
        re::DescriptorSet{m_pipelineLayout.descriptorSetLayout(0)}};
    std::optional<re::StepDoubleBuffered<re::Buffer>> m_branchesBuf;
    std::optional<re::Framebuffer>                    m_framebuffer;
    glm::uvec2                                        m_worldTexSizeTi{};
};

} // namespace rw
