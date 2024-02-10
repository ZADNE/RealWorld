﻿/*!
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

void ChunkGenerator::generateBasicTerrain(const re::CommandBuffer& cmdBuf) {
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateStructurePl);
    m_genPC.storeSegment = 0;
    cmdBuf->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
    cmdBuf->dispatch(k_dispatchSize.x, k_dispatchSize.y, m_chunksPlanned);

    std::array barriers = std::to_array(
        {re::imageMemoryBarrier(
             S::eComputeShader,                              // Src stage mask
             A::eShaderStorageWrite,                         // Src access mask
             S::eComputeShader,                              // Dst stage mask
             A::eShaderStorageWrite | A::eShaderStorageRead, // Dst access mask
             eGeneral,                                       // Old image layout
             eGeneral,                                       // New image layout
             m_tilesTex.image(),
             vk::ImageSubresourceRange{
                 eColor, 0, 1, k_maxParallelChunks * m_genPC.storeSegment, k_maxParallelChunks}
         ),
         re::imageMemoryBarrier(
             S::eComputeShader,      // Src stage mask
             A::eShaderStorageWrite, // Src access mask
             S::eComputeShader,      // Dst stage mask
             A::eShaderStorageRead,  // Dst access mask
             eGeneral,               // Old image layout
             eGeneral,               // New image layout
             m_materialTex.image(),
             vk::ImageSubresourceRange{eColor, 0, 1, 0, k_maxParallelChunks}
         )}
    );
    cmdBuf->pipelineBarrier2({{}, {}, {}, barriers});
}

void ChunkGenerator::consolidateEdges(const re::CommandBuffer& cmdBuf) {
    auto barrier = re::imageMemoryBarrier(
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
        S::eComputeShader,                              // Dst stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Dst access mask
        eGeneral,                                       // Old image layout
        eGeneral,                                       // New image layout
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0, 1, ~0u, k_maxParallelChunks}
    );

    auto pass = [&](glm::ivec2 thresholds, size_t passes) {
        m_genPC.edgeConsolidationPromote = thresholds.x;
        m_genPC.edgeConsolidationReduce  = thresholds.y;
        for (size_t i = 0; i < passes; i++) {
            // Consolidate
            m_genPC.storeSegment = 1 - m_genPC.storeSegment;
            cmdBuf->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
            cmdBuf->dispatch(k_dispatchSize.x, k_dispatchSize.y, m_chunksPlanned);
            // Put barrier for next pass
            barrier.subresourceRange.baseArrayLayer = m_genPC.storeSegment *
                                                      k_maxParallelChunks;
            cmdBuf->pipelineBarrier2({{}, {}, {}, barrier});
        }
    };
    auto doublePass =
        [pass](glm::ivec2 firstThresholds, glm::ivec2 secondThresholds, size_t passes) {
            for (size_t i = 0; i < passes; i++) {
                pass(firstThresholds, 1);
                pass(secondThresholds, 1);
            }
        };

    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_consolidateEdgesPl);
    doublePass({3, 4}, {4, 5}, 4);
}

void ChunkGenerator::selectVariant(const re::CommandBuffer& cmdBuf) {
    // Barrier to wait for the edge consolidation to finish is already there
    m_genPC.storeSegment = 1 - m_genPC.storeSegment;
    cmdBuf->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVariantPl);
    cmdBuf->dispatch(k_dispatchSize.x, k_dispatchSize.y, m_chunksPlanned);
}

} // namespace rw
