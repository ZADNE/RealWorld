/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

namespace rw {

void ChunkGenerator::generateTrees(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateTreesPl);
    commandBuffer.dispatch(1u, 1u, 1u);
}

re::Buffer ChunkGenerator::createTreeTemplatesBuffer() {
    std::vector<Branch> branches;

    branches.emplace_back();

    return re::Buffer{re::BufferCreateInfo{
        .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
        .sizeInBytes = sizeof(Branch) * branches.size(),
        .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
        .initData    = std::as_bytes(std::span{branches})}};
}

} // namespace rw
