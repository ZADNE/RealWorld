/*!
 *  @author    Dubsky Tomas
 */

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/vegetation/BranchAllocRegSB.hpp>
#include <RealWorld/vegetation/BranchSB.hpp>
#include <RealWorld/vegetation/VegSimulator.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::ShaderStageFlagBits;
using D = vk::DescriptorType;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

constexpr glm::uint k_branchBinding      = 0;
constexpr glm::uint k_worldTexAttBinding = 1;
constexpr glm::uint k_worldTexBinding    = 2;

constexpr float k_stepDurationSec = 1.0f / k_physicsStepsPerSecond;

VegSimulator::VegSimulator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{k_branchBinding, D::eStorageBuffer, 1, eVertex | eFragment},
                    {k_worldTexAttBinding, D::eInputAttachment, 1, eFragment},
                    {k_worldTexBinding, D::eStorageImage, 1, eFragment}}},
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

        return re::RenderPassCreateInfo{
            .attachments = attachmentDesc,
            .subpasses   = subpassDescriptions,
            .debugName   = "rw::VegSimulator::unrasterization"};
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

        return re::RenderPassCreateInfo{
            .attachments = attachmentDesc,
            .subpasses   = subpassDescriptions,
            .debugName   = "rw::VegSimulator::rasterization"};
    }()) {
}

void VegSimulator::unrasterizeVegetation(const ActionCmdBuf& acb) {
    auto dbg = acb->createDebugRegion("unrasterization");
    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Prepare rendering to world texture
            beginWorldTextureRenderPass(
                cb, *m_unrasterizationRenderPass, *m_unrasterizationFramebuffer
            );

            // Unrasterize branches from previous step
            cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_unrasterizeBranchesPl);
            cb->drawIndirect(
                *m_branchAllocRegBuf,
                offsetof(BranchAllocRegSB, allocations),
                k_maxBranchAllocCount,
                sizeof(BranchAllocation)
            );
            cb->endRenderPass2(vk::SubpassEndInfo{});
        },
        BufferAccess{
            .name   = BufferTrackName::AllocReg,
            .stage  = S::eDrawIndirect,
            .access = A::eIndirectCommandRead},
        BufferAccess{
            .name   = BufferTrackName::Branch,
            .stage  = S::eVertexShader | S::eFragmentShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite},
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eFragmentShader | S::eColorAttachmentOutput,
            .access = A::eColorAttachmentRead | A::eColorAttachmentWrite}
    );
}

void VegSimulator::rasterizeVegetation(const ActionCmdBuf& acb) {
    auto dbg = acb->createDebugRegion("rasterization");
    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Update push constants
            m_vegDynamicsPC.timeSec += k_stepDurationSec;
            m_vegDynamicsPC.readBuf = re::StepDoubleBufferingState::readIndex();

            // Prepare rendering to world texture
            beginWorldTextureRenderPass(
                cb, *m_rasterizationRenderPass, *m_rasterizationFramebuffer
            );

            // Simulate and rasterize branches
            cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_rasterizeBranchesPl);
            cb->drawIndirect(
                *m_branchAllocRegBuf,
                offsetof(BranchAllocRegSB, allocations),
                k_maxBranchAllocCount,
                sizeof(BranchAllocation)
            );
            cb->endRenderPass2(vk::SubpassEndInfo{});
        },
        BufferAccess{
            .name   = BufferTrackName::AllocReg,
            .stage  = S::eDrawIndirect,
            .access = A::eIndirectCommandRead},
        BufferAccess{
            .name   = BufferTrackName::Branch,
            .stage  = S::eVertexShader | S::eFragmentShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite},
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eFragmentShader | S::eColorAttachmentOutput,
            .access = A::eColorAttachmentRead | A::eColorAttachmentWrite |
                      A::eShaderStorageRead | A::eShaderStorageWrite}
    );
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
    m_branchBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(BranchSB),
        .usage       = eStorageBuffer | eTransferSrc | eTransferDst,
        .debugName   = "rw:VegSimulator::branch"}};

    // Prepare branch allocation register buffer
    auto allocReg = std::make_unique<BranchAllocRegSB>(); // Quite big for stack...
    allocReg->allocations[0] = BranchAllocation{.capacity = k_maxBranchCount};
    m_branchAllocRegBuf      = re::Buffer{re::BufferCreateInfo{
             .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
             .sizeInBytes = sizeof(BranchAllocRegSB),
             .usage       = eStorageBuffer | eIndirectBuffer | eTransferSrc,
             .initData    = re::objectToByteSpan(*allocReg),
             .debugName   = "rw:VegSimulator::branchAllocReg"}};

    // Prepare descriptor
    m_descriptorSet.write(D::eStorageBuffer, k_branchBinding, 0u, m_branchBuf);
    m_descriptorSet.write(
        D::eInputAttachment, k_worldTexAttBinding, 0u, worldTex, vk::ImageLayout::eGeneral
    );
    m_descriptorSet.write(
        D::eStorageImage, k_worldTexBinding, 0u, worldTex, vk::ImageLayout::eGeneral
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

    return VegStorage{
        .branchBuf = m_branchBuf, .branchAllocRegBuf = m_branchAllocRegBuf};
}

void VegSimulator::beginWorldTextureRenderPass(
    const re::CommandBuffer& cb,
    const vk::RenderPass&    renderPass,
    const vk::Framebuffer&   framebuffer
) const {
    cb->beginRenderPass2(
        vk::RenderPassBeginInfo{
            renderPass,
            framebuffer,
            vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}},
            {}},
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}
    );
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    glm::vec2 viewport{m_worldTexSizeTi};
    cb->setViewport(0u, vk::Viewport{0.0f, 0.0, viewport.x, viewport.y, 0.0f, 1.0f});
    cb->setScissor(0u, vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}});
    cb->pushConstants<VegDynamicsPC>(
        *m_pipelineLayout,
        eVertex | eTessellationControl | eTessellationEvaluation,
        0u,
        m_vegDynamicsPC
    );
}

} // namespace rw
