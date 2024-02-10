/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/graphics/commands/CommandBuffer.hpp>

#include <RealWorld/world/VegManager.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

VegManager::VegManager(const re::PipelineLayout& pipelineLayout)
    : m_saveVegetationPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::VegManager::saveVegetation"},
          {.comp = saveVegetation_comp}
      ) {
}

void VegManager::reset() {
    m_bs.forEach([](auto& bs) { bs.reset(); });
}

bool VegManager::hasFreeTransferSpace(glm::ivec2 posAc) const {
    int allocIndex =
        m_regBuf->allocIndexOfTheChunk[posAc.y * k_maxWorldTexSizeCh.x + posAc.x];
    if (allocIndex > 0) {
        auto branchCount = m_regBuf->allocations[allocIndex].branchCount;
        return m_bs->hasFreeTransferSpace(branchCount);
    }
    return true; // No branches allocated for the chunk
}

void VegManager::downloadBranchAllocRegister(
    const re::CommandBuffer& cmdBuf, const re::Buffer& branchBuf
) {
    vk::BufferCopy2 copyRegion{
        offsetof(BranchSB, allocReg), 0ull, sizeof(BranchAllocRegister)};
    cmdBuf->copyBuffer2({*branchBuf, m_regBuf.buffer(), copyRegion});
}

void VegManager::BranchStage::reset() {
    nextUploadSlot   = 0;
    nextDownloadSlot = k_branchStageSlots - 1;
}

bool VegManager::BranchStage::hasFreeTransferSpace(int branchCount) const {
    return (nextUploadSlot + branchCount) <= (nextDownloadSlot - 1);
}

} // namespace rw
