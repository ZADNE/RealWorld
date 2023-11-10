/*!
 *  @author    Dubsky Tomas
 */

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/vegetation/VegSimulator.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::ShaderStageFlagBits;

using D  = vk::DescriptorType;
using S  = vk::PipelineStageFlagBits;
using S2 = vk::PipelineStageFlagBits2;
using A  = vk::AccessFlagBits;
using A2 = vk::AccessFlagBits2;

namespace rw {

constexpr float k_stepDurationSec = 1.0f / k_physicsStepsPerSecond;

VegSimulator::VegSimulator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{0, D::eStorageBuffer, 1, eVertex | eFragment}, // Branch buffer
                    {1, D::eInputAttachment, 1, eFragment}}}, // World texture
              .ranges = {vk::PushConstantRange{
                  eVertex | eTessellationControl | eTessellationEvaluation,
                  0u,
                  sizeof(VegDynamicsPC)}}}
      )
    , m_rasterizationRenderPass([]() {
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
        static std::array subpassDescriptions = std::to_array<vk::SubpassDescription2>(
            {vk::SubpassDescription2{
                 {},
                 vk::PipelineBindPoint::eGraphics,
                 0,
                 worldTexAttachmentRef, // Input attachments
                 worldTexAttachmentRef  // Color attachments
             },
             vk::SubpassDescription2{
                 {},
                 vk::PipelineBindPoint::eGraphics,
                 0,
                 worldTexAttachmentRef, // Input attachments
                 worldTexAttachmentRef  // Color attachments
             }}
        );
        constexpr static auto subpassDependency = vk::SubpassDependency2{
            0,
            1,
            S::eColorAttachmentOutput, // Src stage
            S::eFragmentShader,        // Dst stage
            A::eColorAttachmentWrite,  // Src access
            A::eInputAttachmentRead,   // Dst access
            vk::DependencyFlagBits::eByRegion};

        return vk::RenderPassCreateInfo2{
            vk::RenderPassCreateFlags{},
            attachmentDesc,
            subpassDescriptions,
            subpassDependency};
    }()) {
}

void VegSimulator::step(const vk::CommandBuffer& commandBuffer) {
    // Prepare rendering-to-world-texture state
    commandBuffer.beginRenderPass2(
        vk::RenderPassBeginInfo{
            *m_rasterizationRenderPass,
            *m_framebuffer,
            vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}},
            {}},
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}
    );
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
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
    commandBuffer.pushConstants<VegDynamicsPC>(
        *m_pipelineLayout,
        eVertex | eTessellationControl | eTessellationEvaluation,
        0u,
        m_vegDynamicsPC
    );

    // Unrasterize branches from previous step
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_unrasterizeBranchesPl);
    commandBuffer.drawIndirect(*m_branchBuf, offsetof(BranchSB, vertexCount), 1, 0);
    commandBuffer.nextSubpass2(
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}, vk::SubpassEndInfo{}
    );

    m_vegDynamicsPC.timeSec += k_stepDurationSec;
    m_vegDynamicsPC.readBuf = 1 - m_vegDynamicsPC.readBuf;
    commandBuffer.pushConstants<VegDynamicsPC>(
        *m_pipelineLayout,
        eVertex | eTessellationControl | eTessellationEvaluation,
        0u,
        m_vegDynamicsPC
    );

    // Simulate and rasterize branches
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_rasterizeBranchesPl);
    commandBuffer.drawIndirect(*m_branchBuf, offsetof(BranchSB, vertexCount), 1, 0);
    commandBuffer.endRenderPass2(vk::SubpassEndInfo{});
}

VegSimulator::VegStorage VegSimulator::adoptSave(
    const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh
) {
    // Prepare push constants
    m_worldTexSizeTi               = chToTi(worldTexSizeCh);
    m_vegDynamicsPC.worldTexSizeTi = m_worldTexSizeTi;
    m_vegDynamicsPC.mvpMat =
        glm::ortho<float>(0.0f, m_worldTexSizeTi.x, 0.0f, m_worldTexSizeTi.y);

    // Prepare vegetation buffer
    uint32_t initVegCount{0};
    m_vegBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(glm::uvec2) + k_maxVegCount * sizeof(glm::ivec4),
        .usage       = eStorageBuffer,
        .initData    = re::objectToByteSpan(initVegCount)}};

    // Prepare branch buffer
    vk::DrawIndirectCommand initHeader{0, 1, 0, 0};
    m_branchBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage       = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes       = sizeof(BranchSB),
        .usage             = eStorageBuffer | eIndirectBuffer,
        .initData          = re::objectToByteSpan(initHeader),
        .initDataDstOffset = offsetof(BranchSB, vertexCount)}};

    // Prepare descriptor
    m_descriptorSet.write(D::eStorageBuffer, 0u, 0u, m_branchBuf, 0ull, vk::WholeSize);
    m_descriptorSet.write(
        D::eInputAttachment, 1u, 0u, worldTex, vk::ImageLayout::eGeneral
    );

    // Prepare framebuffer
    m_framebuffer = re::Framebuffer{vk::FramebufferCreateInfo{
        {},
        *m_rasterizationRenderPass,
        worldTex.imageView(),
        m_worldTexSizeTi.x,
        m_worldTexSizeTi.y,
        1u}};

    return VegStorage{.vegBuf = m_vegBuf, .branchBuf = m_branchBuf};
}

} // namespace rw
