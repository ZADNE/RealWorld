/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/constants/tree.hpp>
#include <RealWorld/world/TreeSimulator.hpp>
#include <RealWorld/world/shaders/simulateBranches_comp.hpp>

using enum vk::BufferUsageFlagBits;

namespace rw {

TreeSimulator::TreeSimulator(const re::PipelineLayout& simulationPL)
    : m_simulateBranchesPl{
          {.pipelineLayout = *simulationPL}, {.comp = simulateBranches_comp}} {
}

void TreeSimulator::step(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateBranchesPl);
    commandBuffer.dispatchIndirect(
        **m_branchesBuf, offsetof(BranchesSBHeader, dispatchX)
    );
}

TreeSimulator::Buffers TreeSimulator::adoptSave(const glm::ivec2& worldTexSizeCh) {
    auto maxBranchCount = k_branchesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                          k_branchHeaderSize;

    BranchesSBHeader initHeader{
        .dispatchX          = 0,
        .dispatchY          = 1,
        .dispatchZ          = 1,
        .currentBranchCount = 0,
        .maxBranchCount     = maxBranchCount};

    m_branchesBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(BranchesSBHeader) + sizeof(Branch) * maxBranchCount,
        .usage    = eStorageBuffer | eIndirectBuffer,
        .initData = re::objectToByteSpan(initHeader)}};

    return {.rootsBuf = *m_rootsBuf, .branchesBuf = *m_branchesBuf};
}

} // namespace rw
