/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/mat4x4.hpp>
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
#include <RealWorld/trees/Branch.hpp>
#include <RealWorld/trees/shaders/AllShaders.hpp>

namespace rw {

/**
 * @brief   Simulates swaying and growth of trees
 * @details Rasterizes and unrasterizes the trees to the world texture.
 */
class TreeSimulator {
public:
    TreeSimulator();

    void step(const vk::CommandBuffer& commandBuffer);

    struct Buffers {
        const re::StepDoubleBuffered<re::Buffer>& branchesBuf;
    };

    Buffers adoptSave(const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh);

private:
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
    re::RenderPass     m_rasterizationRenderPass;
    re::Pipeline       m_unrasterizeBranchesPl{
              {.pipelineLayout     = *m_pipelineLayout,
               .topology           = vk::PrimitiveTopology::ePatchList,
               .patchControlPoints = 1,
               .enableBlend        = false,
               .renderPass         = *m_rasterizationRenderPass},
              {.vert = unrasterizeBranches_vert,
               .tesc = tessellateBranches_tesc,
               .tese = tessellateBranches_tese,
               .frag = unrasterizeBranches_frag}};
    re::Pipeline m_rasterizeBranchesPl{
        {.pipelineLayout     = *m_pipelineLayout,
         .topology           = vk::PrimitiveTopology::ePatchList,
         .patchControlPoints = 1,
         .enableBlend        = false,
         .renderPass         = *m_rasterizationRenderPass,
         .subpassIndex       = 1},
        {.vert = rasterizeBranches_vert,
         .tesc = tessellateBranches_tesc,
         .tese = tessellateBranches_tese,
         .frag = rasterizeBranches_frag}};
    re::StepDoubleBuffered<re::DescriptorSet> m_descriptorSets{
        re::DescriptorSet{m_pipelineLayout.descriptorSetLayout(0)},
        re::DescriptorSet{m_pipelineLayout.descriptorSetLayout(0)}};
    std::optional<re::StepDoubleBuffered<re::Buffer>> m_branchesBuf;
    std::optional<re::Framebuffer>                    m_framebuffer;
    glm::uvec2                                        m_worldTexSizeTi{};
};

} // namespace rw
