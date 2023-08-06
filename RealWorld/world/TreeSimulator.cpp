/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/tree.hpp>
#include <RealWorld/world/TreeSimulator.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>

using enum vk::BufferUsageFlagBits;

namespace rw {

TreeSimulator::TreeSimulator(const re::PipelineLayout& simulationPL)
    : m_simulateAndRasterizeBranchesPl{{.pipelineLayout = *simulationPL}, {.comp = simulateFluids_comp}}
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
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute, *m_simulateAndRasterizeBranchesPl
    );
    commandBuffer.dispatchIndirect(
        *m_branchesBuf->write(), offsetof(BranchesSBHeader, branchCount)
    );
}

TreeSimulator::Buffers TreeSimulator::adoptSave(
    const re::Texture& worldTex, const glm::ivec2& worldTexSizeCh
) {
    auto maxBranchCount = k_branchesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                          k_branchHeaderSize;
    glm::uvec2 worldTexSizeTi = chToTi(worldTexSizeCh);

    BranchesSBHeader initHeader{.maxBranchCount = maxBranchCount};

    auto createBranchBuffer = [&] {
        return re::Buffer{re::BufferCreateInfo{
            .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
            .sizeInBytes = sizeof(BranchesSBHeader) + sizeof(Branch) * maxBranchCount,
            .usage    = eStorageBuffer | eIndirectBuffer,
            .initData = re::objectToByteSpan(initHeader)}};
    };

    m_branchesBuf.emplace(createBranchBuffer(), createBranchBuffer());

    m_framebuffer = re::Framebuffer{vk::FramebufferCreateInfo{
        {},
        *m_renderPass,
        worldTex.imageView(),
        worldTexSizeTi.x,
        worldTexSizeTi.y,
        1u}};

    return {*m_branchesBuf};
}

} // namespace rw
