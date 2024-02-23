/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

using enum vk::ImageAspectFlagBits;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

constexpr int        k_groupSize    = 16;
constexpr glm::uvec2 k_dispatchSize = k_genChunkSize / k_groupSize;

void ChunkGenerator::generateBasicTerrain(const re::CommandBuffer& cb) {
    cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateStructurePl);
    m_genPC.storeSegment = 0;
    cb->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
    cb->dispatch(k_dispatchSize.x, k_dispatchSize.y, m_chunksPlanned);

    std::array barriers = std::to_array(
        {re::imageMemoryBarrier(
             S::eComputeShader,                              // Src stage mask
             A::eShaderStorageWrite,                         // Src access mask
             S::eComputeShader,                              // Dst stage mask
             A::eShaderStorageWrite | A::eShaderStorageRead, // Dst access mask
             eGeneral,                                       // Old image layout
             eGeneral,                                       // New image layout
             m_layerTex.image(),
             vk::ImageSubresourceRange{eColor, 0, 1, 0, k_chunkGenSlots * 3}
         ),
         re::imageMemoryBarrier(
             S::eComputeShader,      // Src stage mask
             A::eShaderStorageWrite, // Src access mask
             S::eComputeShader,      // Dst stage mask
             A::eShaderStorageRead,  // Dst access mask
             eGeneral,               // Old image layout
             eGeneral,               // New image layout
             m_materialTex.image(),
             vk::ImageSubresourceRange{eColor, 0, 1, 0, k_chunkGenSlots}
         )}
    );
    cb->pipelineBarrier2({{}, {}, {}, barriers});
}

void ChunkGenerator::consolidateEdges(const re::CommandBuffer& cb) {
    auto barrier = re::imageMemoryBarrier(
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
        S::eComputeShader,                              // Dst stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Dst access mask
        eGeneral,                                       // Old image layout
        eGeneral,                                       // New image layout
        m_layerTex.image(),
        vk::ImageSubresourceRange{eColor, 0, 1, ~0u, k_chunkGenSlots}
    );

    auto pass = [&](glm::ivec2 thresholds, size_t passes) {
        m_genPC.edgeConsolidationPromote = thresholds.x;
        m_genPC.edgeConsolidationReduce  = thresholds.y;
        for (size_t i = 0; i < passes; i++) {
            // Consolidate
            m_genPC.storeSegment = 1 - m_genPC.storeSegment;
            cb->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
            cb->dispatch(k_dispatchSize.x, k_dispatchSize.y, m_chunksPlanned);
            // Put barrier for next pass
            barrier.subresourceRange.baseArrayLayer = m_genPC.storeSegment *
                                                      k_chunkGenSlots;
            cb->pipelineBarrier2({{}, {}, {}, barrier});
        }
    };
    auto doublePass =
        [pass](glm::ivec2 firstThresholds, glm::ivec2 secondThresholds, size_t passes) {
            for (size_t i = 0; i < passes; i++) {
                pass(firstThresholds, 1);
                pass(secondThresholds, 1);
            }
        };

    cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_consolidateEdgesPl);
    doublePass({3, 4}, {4, 5}, 4);
}

void ChunkGenerator::selectVariant(const re::CommandBuffer& cb) {
    // Barrier to wait for the edge consolidation to finish is already there
    m_genPC.storeSegment = 1 - m_genPC.storeSegment;
    cb->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
    cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVariantPl);
    cb->dispatch(k_dispatchSize.x, k_dispatchSize.y, m_chunksPlanned);
}

} // namespace rw
