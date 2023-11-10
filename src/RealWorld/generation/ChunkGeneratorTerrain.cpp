/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

using enum vk::ImageAspectFlagBits;
using enum vk::ShaderStageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

constexpr int        k_groupSize    = 16;
constexpr glm::uvec2 k_dispatchSize = k_genChunkSize / k_groupSize;

void ChunkGenerator::prepareToGenerate(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
}

void ChunkGenerator::generateBasicTerrain(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateStructurePl);
    m_genPC.storeLayer = 0;
    commandBuffer.pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
    commandBuffer.dispatch(k_dispatchSize.x, k_dispatchSize.y, 1u);
}

void ChunkGenerator::consolidateEdges(const vk::CommandBuffer& commandBuffer) {
    auto pass = [&](const glm::ivec2& thresholds, size_t passes) {
        m_genPC.edgeConsolidationPromote = thresholds.x;
        m_genPC.edgeConsolidationReduce  = thresholds.y;
        for (size_t i = 0; i < passes; i++) {
            m_genPC.storeLayer = ~m_genPC.storeLayer & 1;
            // Wait for the previous pass to finish
            auto imageBarrier = worldTexBarrier();
            commandBuffer.pipelineBarrier2({{}, {}, {}, imageBarrier});
            // Consolidate
            commandBuffer.pushConstants<GenerationPC>(
                *m_pipelineLayout, eCompute, 0u, m_genPC
            );
            commandBuffer.dispatch(k_dispatchSize.x, k_dispatchSize.y, 1u);
        }
    };
    auto doublePass = [pass](
                          const glm::ivec2& firstThresholds,
                          const glm::ivec2& secondThresholds,
                          size_t            passes
                      ) {
        for (size_t i = 0; i < passes; i++) {
            pass(firstThresholds, 1);
            pass(secondThresholds, 1);
        }
    };

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_consolidateEdgesPl);
    doublePass({3, 4}, {4, 5}, 4);
}

void ChunkGenerator::selectVariant(const vk::CommandBuffer& commandBuffer) {
    // Wait for the edge consolidation to finish
    auto imageBarrier = worldTexBarrier();
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    // Select variants
    m_genPC.storeLayer = ~m_genPC.storeLayer & 1;
    commandBuffer.pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVariantPl);
    commandBuffer.dispatch(k_dispatchSize.x, k_dispatchSize.y, 1u);
}

} // namespace rw
