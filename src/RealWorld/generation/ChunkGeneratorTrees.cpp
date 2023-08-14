/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

namespace rw {

void ChunkGenerator::generateTrees(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateTreesPl);
    commandBuffer.dispatch(1u, 1u, 1u);
}

} // namespace rw
