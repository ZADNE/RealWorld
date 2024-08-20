/*!
 *  @author    Dubsky Tomas
 */

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/simulation/vegetation/BranchAllocRegSB.hpp>
#include <RealWorld/simulation/vegetation/BranchSB.hpp>
#include <RealWorld/simulation/vegetation/VegSimulator.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageAspectFlagBits;
using D = vk::DescriptorType;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

constexpr glm::uint k_branchBinding       = 0;
constexpr glm::uint k_wallLayerAttBinding = 1;
constexpr glm::uint k_worldTexBinding     = 2;

VegSimulator::VegSimulator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{k_branchBinding, D::eStorageBuffer, 1, eVertex | eFragment},
                    {k_wallLayerAttBinding, D::eInputAttachment, 1, eFragment},
                    {k_worldTexBinding, D::eStorageImage, 1, eFragment}}},
              .ranges = {vk::PushConstantRange{
                  eVertex | eTessellationControl | eTessellationEvaluation | eFragment,
                  0u, sizeof(VegDynamicsPC)
              }}
          }
      )
    , m_rasterizationRenderPass([]() {
        constexpr static vk::AttachmentDescription2 k_attachments{
            {},
            vk::Format::eR16G16Uint,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eLoad,      // Color
            vk::AttachmentStoreOp::eStore,    // Color
            vk::AttachmentLoadOp::eDontCare,  // Stencil
            vk::AttachmentStoreOp::eDontCare, // Stencil
            vk::ImageLayout::eGeneral,        // Initial
            vk::ImageLayout::eGeneral         // Final
        };
        constexpr static vk::AttachmentReference2 k_wallLayerAttRef{
            0, vk::ImageLayout::eGeneral, vk::ImageAspectFlagBits::eColor
        };
        constexpr static auto k_subpasses = std::to_array({vk::SubpassDescription2{
            vk::SubpassDescriptionFlags{}, vk::PipelineBindPoint::eGraphics,
            0u,                 // View mask
            1u,
            &k_wallLayerAttRef, // Input attachments
            1u,
            &k_wallLayerAttRef, // Color attachments
        }});

        return re::RenderPassCreateInfo{
            .attachments = {&k_attachments, 1},
            .subpasses   = k_subpasses,
            .debugName   = "rw::VegSimulator::rasterization"
        };
    }()) {
}

void VegSimulator::unrasterizeVegetation(const ActionCmdBuf& acb) {
    auto dbg = acb->createDebugRegion("unrasterization");
    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Prepare rendering to world texture
            beginWorldTextureRenderPass(cb);

            // Unrasterize branches from previous step
            cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_unrasterizeBranchesPl);
            cb->drawIndirect(
                *m_branchAllocRegBuf, offsetof(BranchAllocRegSB, allocations),
                k_maxBranchAllocCount, sizeof(BranchAllocation)
            );
            cb->endRenderPass2(vk::SubpassEndInfo{});
        },
        BufferAccess{
            .name   = BufferTrackName::AllocReg,
            .stage  = S::eDrawIndirect,
            .access = A::eIndirectCommandRead
        },
        BufferAccess{
            .name   = BufferTrackName::Branch,
            .stage  = S::eVertexShader | S::eFragmentShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        },
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eFragmentShader | S::eColorAttachmentOutput,
            .access = A::eInputAttachmentRead | A::eColorAttachmentRead |
                      A::eColorAttachmentWrite
        }
    );
}

void VegSimulator::rasterizeVegetation(const ActionCmdBuf& acb, float timeSec) {
    auto dbg = acb->createDebugRegion("rasterization");
    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Update push constants
            m_vegDynamicsPC.timeSec = timeSec;
            m_vegDynamicsPC.readBuf = re::StepDoubleBufferingState::readIndex();

            // Prepare rendering to world texture
            beginWorldTextureRenderPass(cb);

            // Simulate and rasterize branches
            cb->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_rasterizeBranchesPl);
            cb->drawIndirect(
                *m_branchAllocRegBuf, offsetof(BranchAllocRegSB, allocations),
                k_maxBranchAllocCount, sizeof(BranchAllocation)
            );
            cb->endRenderPass2(vk::SubpassEndInfo{});
        },
        BufferAccess{
            .name   = BufferTrackName::AllocReg,
            .stage  = S::eDrawIndirect,
            .access = A::eIndirectCommandRead
        },
        BufferAccess{
            .name   = BufferTrackName::Branch,
            .stage  = S::eVertexShader | S::eFragmentShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        },
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eFragmentShader | S::eColorAttachmentOutput,
            .access = A::eInputAttachmentRead | A::eColorAttachmentRead |
                      A::eColorAttachmentWrite | A::eShaderStorageRead |
                      A::eShaderStorageWrite
        }
    );
}

VegSimulator::VegStorage VegSimulator::adoptSave(
    const re::Texture& worldTex, glm::ivec2 worldTexSizeCh
) {
    // Prepare push constants
    m_worldTexSizeTi               = chToTi(worldTexSizeCh);
    m_vegDynamicsPC.worldTexSizeTi = m_worldTexSizeTi;
    m_vegDynamicsPC.worldTexMaskTi = m_worldTexSizeTi - 1u;
    m_vegDynamicsPC.mvpMat =
        glm::ortho<float>(0.0f, m_worldTexSizeTi.x, 0.0f, m_worldTexSizeTi.y);

    // Prepare branch buffer
    m_branchBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(BranchSB),
        .usage       = eStorageBuffer | eTransferSrc | eTransferDst,
        .debugName   = "rw:VegSimulator::branch"
    }};

    // Prepare branch allocation register buffer
    auto allocReg = std::make_unique<BranchAllocRegSB>(); // Quite big for stack...
    allocReg->allocations[0] = BranchAllocation{.capacity = k_maxBranchCount};
    m_branchAllocRegBuf      = re::Buffer{re::BufferCreateInfo{
             .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
             .sizeInBytes = sizeof(BranchAllocRegSB),
             .usage       = eStorageBuffer | eIndirectBuffer | eTransferSrc,
             .initData    = re::objectToByteSpan(*allocReg),
             .debugName   = "rw:VegSimulator::branchAllocReg"
    }};

    // Prepare framebuffers
    m_wallLayerImageView = re::ImageView{vk::ImageViewCreateInfo{
        {},
        worldTex.image(),
        vk::ImageViewType::e2D,
        vk::Format::eR16G16Uint,
        {},
        {eColor, 0, 1, std::to_underlying(TileLayer::Wall), 1}
    }};

    m_rasterizationFramebuffer = re::Framebuffer{vk::FramebufferCreateInfo{
        {},
        *m_rasterizationRenderPass,
        *m_wallLayerImageView,
        m_worldTexSizeTi.x,
        m_worldTexSizeTi.y,
        1u
    }};

    // Prepare descriptor
    m_descriptorSet.write(D::eStorageBuffer, k_branchBinding, 0u, m_branchBuf);
    m_descriptorSet.write(
        D::eInputAttachment, k_wallLayerAttBinding, 0u,
        vk::DescriptorImageInfo{nullptr, *m_wallLayerImageView, vk::ImageLayout::eGeneral}
    );
    m_descriptorSet.write(
        D::eStorageImage, k_worldTexBinding, 0u, worldTex, vk::ImageLayout::eGeneral
    );

    return VegStorage{.branchBuf = m_branchBuf, .branchAllocRegBuf = m_branchAllocRegBuf};
}

void VegSimulator::beginWorldTextureRenderPass(const re::CommandBuffer& cb) const {
    cb->beginRenderPass2(
        vk::RenderPassBeginInfo{
            *m_rasterizationRenderPass,
            *m_rasterizationFramebuffer,
            vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}},
            {}
        },
        vk::SubpassBeginInfo{vk::SubpassContents::eInline}
    );
    cb->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u,
        *m_descriptorSet, {}
    );
    glm::vec2 viewport{m_worldTexSizeTi};
    cb->setViewport(0u, vk::Viewport{0.0f, 0.0, viewport.x, viewport.y, 0.0f, 1.0f});
    cb->setScissor(0u, vk::Rect2D{{0, 0}, {m_worldTexSizeTi.x, m_worldTexSizeTi.y}});
    cb->pushConstants<VegDynamicsPC>(
        *m_pipelineLayout,
        eVertex | eTessellationControl | eTessellationEvaluation | eFragment,
        0u, m_vegDynamicsPC
    );
}

} // namespace rw
