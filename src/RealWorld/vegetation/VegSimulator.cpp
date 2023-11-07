/*!
 *  @author    Dubsky Tomas
 */

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/vegetation/VegSimulator.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::ShaderStageFlagBits;

using D  = vk::DescriptorType;
using S  = vk::PipelineStageFlagBits;
using S2 = vk::PipelineStageFlagBits2;
using A  = vk::AccessFlagBits;
using A2 = vk::AccessFlagBits2;

namespace rw {

constexpr float          k_stepDurationSec    = 1.0f / k_physicsStepsPerSecond;
constexpr int            k_branchesPerChunk   = 16;
constexpr vk::DeviceSize k_branchRasterSpace  = 64;
constexpr uint32_t       k_vegetationPerChunk = 2;

VegSimulator::VegSimulator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{0, D::eStorageBuffer, 1, eVertex}, // Branch-vector buffer write
                    {1, D::eStorageBuffer, 1, eVertex}, // Branch-vector buffer read
                    {2, D::eInputAttachment, 1, eFragment}, // World texture
                    {3, D::eStorageBuffer, 1, eFragment}}}, // Branch-raster buffer
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
    m_vegDynamicsPC.timeSec += k_stepDurationSec;
    commandBuffer.beginRenderPass2(
        vk::RenderPassBeginInfo{
            *m_rasterizationRenderPass,
            *m_framebuffer,
            vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}},
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
    commandBuffer.drawIndirect(
        *m_branchVectorBuf.read(), offsetof(BranchesSBHeader, vertexCount), 1, 0
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
        *m_branchVectorBuf.read(), offsetof(BranchesSBHeader, vertexCount), 1, 0
    );
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
    int maxBranchCount = k_branchesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                         k_branchHeaderSize;

    // Prepare vegetation buffer
    uint32_t initVegCount = 0;
    uint32_t maxVegCount  = k_vegetationPerChunk * worldTexSizeCh.x *
                           worldTexSizeCh.y;
    m_vegBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(glm::uvec2) + maxVegCount * sizeof(glm::uvec4),
        .usage       = eStorageBuffer,
        .initData    = re::objectToByteSpan(initVegCount)}};

    { // Prepare branch-vector buffer
        auto createBranchBuffer = [&] {
            BranchesSBHeader initHeader{.maxBranchCount = maxBranchCount};
            return re::Buffer{re::BufferCreateInfo{
                .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
                .sizeInBytes = sizeof(BranchesSBHeader) +
                               sizeof(Branch) * maxBranchCount,
                .usage = eStorageBuffer | eIndirectBuffer | eTransferSrc | eTransferDst,
                .initData = re::objectToByteSpan(initHeader)}};
        };
        m_branchVectorBuf = {createBranchBuffer(), createBranchBuffer()};
    }

    // Prepare branch-raster texture
    m_branchRasterBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = maxBranchCount * k_branchRasterSpace,
        .usage       = eStorageBuffer}};

    { // Prepare descriptors
        auto writeDescriptor = [&](re::DescriptorSet& set,
                                   re::Buffer&        first,
                                   re::Buffer&        second) {
            set.write(D::eStorageBuffer, 0u, 0u, first, 0ull, vk::WholeSize);
            set.write(D::eStorageBuffer, 1u, 0u, second, 0ull, vk::WholeSize);
            set.write(D::eInputAttachment, 2u, 0u, worldTex, vk::ImageLayout::eGeneral);
            set.write(D::eStorageBuffer, 3u, 0u, m_branchRasterBuf, 0ull, vk::WholeSize);
        };
        writeDescriptor(
            m_descriptorSets[0], m_branchVectorBuf[0], m_branchVectorBuf[1]
        );
        writeDescriptor(
            m_descriptorSets[1], m_branchVectorBuf[1], m_branchVectorBuf[0]
        );
    }

    // Prepare framebuffer
    m_framebuffer = re::Framebuffer{vk::FramebufferCreateInfo{
        {},
        *m_rasterizationRenderPass,
        worldTex.imageView(),
        m_worldTexSizeTi.x,
        m_worldTexSizeTi.y,
        1u}};

    return {m_vegBuf, maxVegCount, m_branchVectorBuf, m_branchRasterBuf};
}

} // namespace rw
