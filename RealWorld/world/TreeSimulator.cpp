/*!
 *  @author    Dubsky Tomas
 */

#include <RealWorld/constants/tree.hpp>
#include <RealWorld/world/TreeSimulator.hpp>
#include <RealWorld/world/shaders/simulateTrees_comp.hpp>

using enum vk::BufferUsageFlagBits;

namespace rw {

TreeSimulator::TreeSimulator(const re::PipelineLayout& simulationPL)
    : m_simulateTreesPl{
          {.pipelineLayout = *simulationPL}, {.comp = simulateTrees_comp}} {
}

void TreeSimulator::step(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_simulateTreesPl);
    commandBuffer.dispatchIndirect(
        **m_branchesBuf, offsetof(BranchesSBHeader, dispatchX)
    );
}

TreeSimulator::Buffers TreeSimulator::adoptSave(const glm::ivec2& worldTexSizeCh) {
    auto maxRootCount = k_rootsPerChunk * worldTexSizeCh.x * worldTexSizeCh.y;
    auto maxBranchCount = k_branchesPerChunk * worldTexSizeCh.x * worldTexSizeCh.y -
                          k_branchHeaderSize;

    BranchesSBHeader initHeader{
        .maxRootCount = maxRootCount, .maxBranchCount = maxBranchCount};

    m_rootsBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(Root) * maxRootCount,
        .usage       = eStorageBuffer}};

    m_branchesBuf = re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(BranchesSBHeader) + sizeof(Branch) * maxBranchCount,
        .usage    = eStorageBuffer | eIndirectBuffer,
        .initData = re::objectToByteSpan(initHeader)}};

    return {.rootsBuf = *m_rootsBuf, .branchesBuf = *m_branchesBuf};
}

} // namespace rw
