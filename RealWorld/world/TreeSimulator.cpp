/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/constants/tree.hpp>
#include <RealWorld/world/TreeSimulator.hpp>
#include <RealWorld/world/shaders/simulateTrees_comp.hpp>

using enum vk::BufferUsageFlagBits;

namespace rw {

TreeSimulator::TreeSimulator(const re::PipelineLayout& simulationPL)
    : m_simulateAndRasterizeBranchesPl{
          {.pipelineLayout = *simulationPL}, {.comp = simulateTrees_comp}} {
}

void TreeSimulator::step(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute, *m_simulateAndRasterizeBranchesPl
    );
    commandBuffer.dispatchIndirect(
        *m_branchesBuf->write(), offsetof(BranchesSBHeader, branchCount)
    );
}

TreeSimulator::Buffers TreeSimulator::adoptSave(const glm::ivec2& worldTexSizeCh) {
    auto maxBranchCount = k_branchesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                          k_branchHeaderSize;

    BranchesSBHeader initHeader{.maxBranchCount = maxBranchCount};

    auto createBranchBuffer = [&] {
        return re::Buffer{re::BufferCreateInfo{
            .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
            .sizeInBytes = sizeof(BranchesSBHeader) + sizeof(Branch) * maxBranchCount,
            .usage    = eStorageBuffer | eIndirectBuffer,
            .initData = re::objectToByteSpan(initHeader)}};
    };

    m_branchesBuf.emplace(createBranchBuffer(), createBranchBuffer());

    return {*m_branchesBuf};
}

} // namespace rw
