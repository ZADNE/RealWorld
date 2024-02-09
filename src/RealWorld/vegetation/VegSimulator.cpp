/*!
 *  @author    Dubsky Tomas
 */

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/vegetation/BranchSB.hpp>
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
    , m_unrasterizationRenderPass([]() {
        constexpr static auto attachmentDesc = vk::AttachmentDescription2{
            // The world texture attachment
            {},
            vk::Format::eR8G8B8A8Uint,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eLoad,       // Color
            vk::AttachmentStoreOp::eStore,     // Color
            vk::AttachmentLoadOp::eDontCare,   // Stencil
            vk::AttachmentStoreOp::eDontCare,  // Stencil
            vk::ImageLayout::eReadOnlyOptimal, // Initial
            vk::ImageLayout::eGeneral          // Final
        };
        constexpr static auto worldTexAttachmentRef = vk::AttachmentReference2{
            0, vk::ImageLayout::eGeneral, vk::ImageAspectFlagBits::eColor};
        static std::array subpassDescriptions =
            std::to_array<vk::SubpassDescription2>({vk::SubpassDescription2{
                {},
                vk::PipelineBindPoint::eGraphics,
                0,
                worldTexAttachmentRef, // Input attachments
                worldTexAttachmentRef  // Color attachments
            }});
        constexpr static auto subpassDependency = vk::SubpassDependency2{
            vk::SubpassExternal,
            0,
            S::eFragmentShader | S::eComputeShader,             // Src stage
            S::eFragmentShader | S::eColorAttachmentOutput,     // Dst stage
            A::eShaderRead,                                     // Src access
            A::eColorAttachmentRead | A::eColorAttachmentWrite, // Dst access
            vk::DependencyFlagBits::eByRegion};

        return re::RenderPassCreateInfo{
            .attachments  = attachmentDesc,
            .subpasses    = subpassDescriptions,
            .dependencies = subpassDependency,
            .debugName    = "rw::VegSimulator::unrasterization"};
    }())
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
        static std::array subpassDescriptions =
            std::to_array<vk::SubpassDescription2>({vk::SubpassDescription2{
                {},
                vk::PipelineBindPoint::eGraphics,
                0,
                worldTexAttachmentRef, // Input attachments
                worldTexAttachmentRef  // Color attachments
            }});
        constexpr static auto subpassDependency = vk::SubpassDependency2{
            vk::SubpassExternal,
            0,
            S::eTransfer,                                       // Src stage
            S::eFragmentShader | S::eColorAttachmentOutput,     // Dst stage
            A::eTransferWrite,                                  // Src access
            A::eColorAttachmentRead | A::eColorAttachmentWrite, // Dst access
            vk::DependencyFlagBits::eByRegion};

        return re::RenderPassCreateInfo{
            .attachments  = attachmentDesc,
            .subpasses    = subpassDescriptions,
            .dependencies = subpassDependency,
            .debugName    = "rw::VegSimulator::rasterization"};
    }()) {
}

void VegSimulator::unrasterizeVegetation(const re::CommandBuffer& cmdBuf) {
    auto dbg = cmdBuf.createDebugRegion("unrasterization");

    // Prepare rendering to world texture
    beginWorldTextureRenderPass(
        cmdBuf, *m_unrasterizationRenderPass, *m_unrasterizationFramebuffer
    );

    // Unrasterize branches from previous step
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_unrasterizeBranchesPl);
    cmdBuf->drawIndirect(
        *m_branchBuf,
        offsetof(BranchSB, allocReg.allocations),
        k_maxBranchAllocCount,
        sizeof(BranchAllocation)
    );
    cmdBuf->endRenderPass2(vk::SubpassEndInfo{});
}

void VegSimulator::rasterizeVegetation(const re::CommandBuffer& cmdBuf) {
    auto dbg = cmdBuf.createDebugRegion("rasterization");

    // Update push constants
    m_vegDynamicsPC.timeSec += k_stepDurationSec;
    m_vegDynamicsPC.readBuf = 1 - m_vegDynamicsPC.readBuf;

    // Prepare rendering to world texture
    beginWorldTextureRenderPass(
        cmdBuf, *m_rasterizationRenderPass, *m_rasterizationFramebuffer
    );

    // Simulate and rasterize branches
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_rasterizeBranchesPl);
    cmdBuf->drawIndirect(
        *m_branchBuf,
        offsetof(BranchSB, allocReg.allocations),
        k_maxBranchAllocCount,
        sizeof(BranchAllocation)
    );
    cmdBuf->endRenderPass2(vk::SubpassEndInfo{});
}

VegSimulator::VegStorage VegSimulator::adoptSave(
    const re::Texture& worldTex, glm::ivec2 worldTexSizeCh
) {
    // Prepare push constants
    m_worldTexSizeTi               = chToTi(worldTexSizeCh);
    m_vegDynamicsPC.worldTexSizeTi = m_worldTexSizeTi;
    m_vegDynamicsPC.mvpMat =
        glm::ortho<float>(0.0f, m_worldTexSizeTi.x, 0.0f, m_worldTexSizeTi.y);

    // Prepare branch buffer
    BranchAllocRegister allocReg{};
    allocReg.allocations[0] =
        BranchAllocation{.firstBranch = 0, .capacity = k_maxBranchCount};

    m_branchBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage       = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes       = sizeof(BranchSB),
        .usage             = eStorageBuffer | eIndirectBuffer,
        .initData          = re::objectToByteSpan(allocReg),
        .initDataDstOffset = offsetof(BranchSB, allocReg),
        .debugName         = "rw:VegSimulator::branch"}};

    // Prepare descriptor
    m_descriptorSet.write(D::eStorageBuffer, 0u, 0u, m_branchBuf, 0ull, vk::WholeSize);
    m_descriptorSet.write(
        D::eInputAttachment, 1u, 0u, worldTex, vk::ImageLayout::eGeneral
    );

    { // Prepare framebuffers
        vk::FramebufferCreateInfo createInfo{
            {},
            nullptr,
            worldTex.imageView(),
            m_worldTexSizeTi.x,
            m_worldTexSizeTi.y,
            1u};
        createInfo.renderPass        = *m_unrasterizationRenderPass;
        m_unrasterizationFramebuffer = re::Framebuffer{createInfo};
        createInfo.renderPass        = *m_rasterizationRenderPass;
        m_rasterizationFramebuffer   = re::Framebuffer{createInfo};
    }

    return VegStorage{.branchBuf = m_branchBuf};
}

void VegSimulator::beginWorldTextureRenderPass(
    const re::CommandBuffer& cmdBuf,
    const vk::RenderPass&    renderPass,
    const vk::Framebuffer&   framebuffer
) const {
    cmdBuf->beginRenderPass2(
        vk::RenderPassBeginInfo{
            renderPass,
            framebuffer,
            vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}},
            {}},
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}
    );
    cmdBuf->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    glm::vec2 viewport{m_worldTexSizeTi};
    cmdBuf->setViewport(
        0u, vk::Viewport{0.0f, 0.0, viewport.x, viewport.y, 0.0f, 1.0f}
    );
    cmdBuf->setScissor(
        0u, vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}}
    );
    cmdBuf->pushConstants<VegDynamicsPC>(
        *m_pipelineLayout,
        eVertex | eTessellationControl | eTessellationEvaluation,
        0u,
        m_vegDynamicsPC
    );
}

} // namespace rw
