/*!
 *  @author    Dubsky Tomas
 */

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/tree.hpp>
#include <RealWorld/trees/TreeSimulator.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::ShaderStageFlagBits;

using D  = vk::DescriptorType;
using S  = vk::PipelineStageFlagBits;
using S2 = vk::PipelineStageFlagBits2;
using A  = vk::AccessFlagBits;
using A2 = vk::AccessFlagBits2;

namespace rw {

constexpr float k_stepDurationSec = 1.0f / k_physicsStepsPerSecond;

TreeSimulator::TreeSimulator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{0, D::eStorageBuffer, 1, eVertex},
                    {1, D::eStorageBuffer, 1, eVertex},
                    {2, D::eStorageImage, 1, eFragment}}},
              .ranges = {vk::PushConstantRange{
                  eVertex | eTessellationEvaluation, 0u, sizeof(TreeDynamicsPC)}}}
      )
    , m_rasterizationRenderPass([]() {
        static std::array subpassDescriptions = std::to_array<vk::SubpassDescription2>(
            {vk::SubpassDescription2{
                 {},
                 vk::PipelineBindPoint::eGraphics,
                 0,
                 {}, // Input attachments
                 {}  // Color attachments
             },
             vk::SubpassDescription2{
                 {},
                 vk::PipelineBindPoint::eGraphics,
                 0,
                 {}, // Input attachments
                 {}  // Color attachments
             }}
        );
        constexpr static auto subpassDependency = vk::SubpassDependency2{
            0,
            1,
            S::eFragmentShader, // Src stage
            S::eFragmentShader, // Dst stage
            A::eShaderWrite,    // Src access
            A::eShaderRead,     // Dst access
            vk::DependencyFlagBits::eByRegion};

        return vk::RenderPassCreateInfo2{
            vk::RenderPassCreateFlags{}, {}, subpassDescriptions, subpassDependency};
    }()) {
}

void TreeSimulator::step(const vk::CommandBuffer& commandBuffer) {
    m_treeDynamicsPC.timeSec += k_stepDurationSec;
    commandBuffer.beginRenderPass2(
        vk::RenderPassBeginInfo{
            *m_rasterizationRenderPass,
            **m_framebuffer,
            vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x * 2, m_worldTexSizeTi.y * 2}},
            {}},
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}
    );
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *m_pipelineLayout,
        0u,
        *m_descriptorSets.write(),
        {}
    );
    glm::vec2 sz{glm::vec2{m_worldTexSizeTi} * 2.0f};
    commandBuffer.setViewport(0u, vk::Viewport{0.0f, 0.0f, sz.x, sz.y, 0.0f, 1.0f});
    commandBuffer.setScissor(
        0u,
        vk::Rect2D{
            vk::Offset2D{0, 0},
            vk::Extent2D{m_worldTexSizeTi.x * 2, m_worldTexSizeTi.y * 2}}
    );
    commandBuffer.pushConstants<TreeDynamicsPC>(
        *m_pipelineLayout, eVertex | eTessellationEvaluation, 0u, m_treeDynamicsPC
    );
    // Unrasterize branches from previous step
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_unrasterizeBranchesPl);
    commandBuffer.drawIndirect(
        *m_branchesBuf->read(), offsetof(BranchesSBHeader, vertexCount), 1, 0
    );

    commandBuffer.nextSubpass2(
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}, vk::SubpassEndInfo{}
    );

    // Simulate and rasterize branches
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *m_pipelineLayout,
        0u,
        *m_descriptorSets.read(),
        {}
    );
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_rasterizeBranchesPl);
    commandBuffer.drawIndirect(
        *m_branchesBuf->read(), offsetof(BranchesSBHeader, vertexCount), 1, 0
    );
    commandBuffer.endRenderPass2(vk::SubpassEndInfo{});
}

TreeSimulator::Buffers TreeSimulator::adoptSave(
    const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh
) {
    auto maxBranchCount = k_branchesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                          k_branchHeaderSize;
    m_worldTexSizeTi                = chToTi(worldTexSizeCh);
    m_treeDynamicsPC.worldTexSizeTi = m_worldTexSizeTi;
    m_treeDynamicsPC.mvpMat         = glm::ortho<float>(
        m_worldTexSizeTi.x * -0.5f,
        m_worldTexSizeTi.x * 1.5f,
        m_worldTexSizeTi.y * -0.5f,
        m_worldTexSizeTi.y * 1.5f
    );

    BranchesSBHeader initHeader{.maxBranchCount = maxBranchCount};

    auto createBranchBuffer = [&] {
        return re::Buffer{re::BufferCreateInfo{
            .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
            .sizeInBytes = sizeof(BranchesSBHeader) + sizeof(Branch) * maxBranchCount,
            .usage = eStorageBuffer | eIndirectBuffer | eTransferSrc | eTransferDst,
            .initData = re::objectToByteSpan(initHeader)}};
    };

    m_branchesBuf.emplace(createBranchBuffer(), createBranchBuffer());
    auto writeDescriptor =
        [&](re::DescriptorSet& set, re::Buffer& first, re::Buffer& second) {
            set.write(D::eStorageBuffer, 0u, 0u, first, 0ull, vk::WholeSize);
            set.write(D::eStorageBuffer, 1u, 0u, second, 0ull, vk::WholeSize);
            set.write(D::eStorageImage, 2u, 0u, worldTex, vk::ImageLayout::eGeneral);
        };
    writeDescriptor(m_descriptorSets[0], (*m_branchesBuf)[0], (*m_branchesBuf)[1]);
    writeDescriptor(m_descriptorSets[1], (*m_branchesBuf)[1], (*m_branchesBuf)[0]);

    m_framebuffer = re::Framebuffer{vk::FramebufferCreateInfo{
        {},
        *m_rasterizationRenderPass,
        {}, // No attachments
        m_worldTexSizeTi.x * 2,
        m_worldTexSizeTi.y * 2,
        1u}};

    return {*m_branchesBuf};
}

} // namespace rw
