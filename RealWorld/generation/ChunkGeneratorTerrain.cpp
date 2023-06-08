﻿/*!
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
        vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descSet, {}
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
            auto imageBarrier = stepBarrier();
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
    auto imageBarrier = stepBarrier();
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    // Select variants
    m_genPC.storeLayer = ~m_genPC.storeLayer & 1;
    commandBuffer.pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_genPC);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVariantPl);
    commandBuffer.dispatch(k_dispatchSize.x, k_dispatchSize.y, 1u);
}

void ChunkGenerator::finishGeneration(
    const vk::CommandBuffer& commandBuffer,
    const re::Texture&       dstTex,
    const glm::ivec2&        dstOffset
) {
    // Wait for the generation to finish
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
        S::eTransfer,                                   // Dst stage mask
        A::eTransferRead,                               // Dst access mask
        vk::ImageLayout::eGeneral,                      // Old image layout
        vk::ImageLayout::eGeneral,                      // New image layout
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, // Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0u, 1u, m_genPC.storeLayer, 1u}};
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    // Copy the generated chunk to the world texture
    commandBuffer.copyImage(
        m_tilesTex.image(),
        vk::ImageLayout::eGeneral, // Src image
        dstTex.image(),
        vk::ImageLayout::eGeneral, // Dst image
        vk::ImageCopy{
            vk::ImageSubresourceLayers{eColor, 0u, m_genPC.storeLayer, 1u}, // Src subresource
            vk::Offset3D{k_genBorderWidth, k_genBorderWidth, 0}, // Src offset
            vk::ImageSubresourceLayers{eColor, 0u, 0u, 1u}, // Dst subresource
            vk::Offset3D{dstOffset.x, dstOffset.y, 0},      // Dst offset
            vk::Extent3D{iChunkTi.x, iChunkTi.y, 1}         // Copy Extent
        }
    );
}

} // namespace rw