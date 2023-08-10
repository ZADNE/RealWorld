/*!
 *  @author    Dubsky Tomas
 */

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/tree.hpp>
#include <RealWorld/world/TreeSimulator.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::ShaderStageFlagBits;

using D = vk::DescriptorType;

namespace rw {

constexpr float k_stepDurationSec = 1.0f / k_physicsStepsPerSecond;

TreeSimulator::TreeSimulator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{0, D::eStorageBuffer, 1, eVertex},
                    {1, D::eStorageBuffer, 1, eVertex}}},
              .ranges = {vk::PushConstantRange{
                  eVertex | eTessellationEvaluation, 0u, sizeof(TreeDynamicsPC)}}}
      )
    , m_renderPass([]() {
        constexpr static auto attachmentDesc = vk::AttachmentDescription2{
            // The world texture attachment
            {},
            vk::Format::eR8G8B8A8Uint,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eLoad,      // Color
            vk::AttachmentStoreOp::eStore,    // Color
            vk::AttachmentLoadOp::eDontCare,  // Stencil
            vk::AttachmentStoreOp::eDontCare, // Stencil
            vk::ImageLayout::eGeneral,        // Initial
            vk::ImageLayout::eGeneral         // Final
        };
        constexpr static auto worldTexAttachmentRef = vk::AttachmentReference2{
            0, vk::ImageLayout::eGeneral, vk::ImageAspectFlagBits::eColor};
        static auto subpassDescription = vk::SubpassDescription2{
            {},
            vk::PipelineBindPoint::eGraphics,
            0,
            worldTexAttachmentRef, // Input
            worldTexAttachmentRef  // Color
        };
        return vk::RenderPassCreateInfo2{
            vk::RenderPassCreateFlags{}, attachmentDesc, subpassDescription};
    }()) {
}

void TreeSimulator::step(const vk::CommandBuffer& commandBuffer) {
    m_treeDynamicsPC.timeSec += k_stepDurationSec;
    commandBuffer.beginRenderPass2(
        vk::RenderPassBeginInfo{
            *m_renderPass,
            **m_framebuffer,
            vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}},
            {}},
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}
    );
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *m_pipelineLayout,
        0u,
        *m_descriptorSets.read(),
        {}
    );

    glm::vec2 viewport{m_worldTexSizeTi};
    commandBuffer.setViewport(
        0u, vk::Viewport{0.0f, 0.0, viewport.x, viewport.y, 0.0f, 1.0f}
    );
    commandBuffer.setScissor(
        0u,
        vk::Rect2D{
            {0, 0},                                  // x, y
            {m_worldTexSizeTi.x, m_worldTexSizeTi.y} // width, height
        }
    );
    commandBuffer.pushConstants<TreeDynamicsPC>(
        *m_pipelineLayout, eVertex | eTessellationEvaluation, 0u, m_treeDynamicsPC
    );
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, *m_simulateAndRasterizeBranchesPl
    );
    commandBuffer.drawIndirect(
        *m_branchesBuf->write(), offsetof(BranchesSBHeader, vertexCount), 1, 0
    );
    commandBuffer.endRenderPass2(vk::SubpassEndInfo{});
}

TreeSimulator::Buffers TreeSimulator::adoptSave(
    const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh
) {
    auto maxBranchCount = k_branchesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                          k_branchHeaderSize;
    m_worldTexSizeTi = chToTi(worldTexSizeCh);
    m_treeDynamicsPC.worldTexSizeTi = m_worldTexSizeTi;
    m_treeDynamicsPC.mvpMat =
        glm::ortho<float>(0.0f, m_worldTexSizeTi.x, 0.0f, m_worldTexSizeTi.y);

    BranchesSBHeader initHeader{.maxBranchCount = maxBranchCount};

    auto createBranchBuffer = [&] {
        return re::Buffer{re::BufferCreateInfo{
            .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
            .sizeInBytes = sizeof(BranchesSBHeader) + sizeof(Branch) * maxBranchCount,
            .usage    = eStorageBuffer | eIndirectBuffer,
            .initData = re::objectToByteSpan(initHeader)}};
    };

    m_branchesBuf.emplace(createBranchBuffer(), createBranchBuffer());
    auto writeDescriptor =
        [&](re::DescriptorSet& set, re::Buffer& first, re::Buffer& second) {
            set.write(D::eStorageBuffer, 0u, 0u, first, 0ull, VK_WHOLE_SIZE);
            set.write(D::eStorageBuffer, 1u, 0u, second, 0ull, VK_WHOLE_SIZE);
        };
    writeDescriptor(m_descriptorSets[0], (*m_branchesBuf)[0], (*m_branchesBuf)[1]);
    writeDescriptor(m_descriptorSets[1], (*m_branchesBuf)[1], (*m_branchesBuf)[0]);

    m_framebuffer = re::Framebuffer{vk::FramebufferCreateInfo{
        {},
        *m_renderPass,
        worldTex.imageView(),
        m_worldTexSizeTi.x,
        m_worldTexSizeTi.y,
        1u}};

    return {*m_branchesBuf};
}

} // namespace rw
