/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGeneratorCS.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

constexpr int GEN_CS_GROUP_SIZE = 16;
constexpr glm::uvec2 DISPATCH_SIZE = GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE;

using enum vk::ImageAspectFlagBits;
using enum vk::ShaderStageFlagBits;

ChunkGeneratorCS::ChunkGeneratorCS() {
    /*m_tilesTex[0].bind(TEX_UNIT_GEN_TILES[0]);
    m_tilesTex[1].bind(TEX_UNIT_GEN_TILES[1]);
    m_materialGenTex.bind(TEX_UNIT_GEN_MATERIAL);

    m_tilesTex[0].bindImage(IMG_UNIT_GEN_TILES[0], 0, RE::ImageAccess::READ_WRITE);
    m_tilesTex[1].bindImage(IMG_UNIT_GEN_TILES[1], 0, RE::ImageAccess::READ_WRITE);
    m_materialGenTex.bindImage(IMG_UNIT_GEN_MATERIAL, 0, RE::ImageAccess::READ_WRITE);*/
}

void ChunkGeneratorCS::prepareToGenerate() {
    m_commandBuffer->begin({});
}

void ChunkGeneratorCS::generateBasicTerrain() {
    m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateStructurePl);
    m_commandBuffer->pushConstants<PushConstants>(m_generateStructurePl.pipelineLayout(), eCompute, 0u, m_pushConstants);
    m_commandBuffer->dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
}

void ChunkGeneratorCS::consolidateEdges() {
    glm::uint cycleN = 0u;
    auto pass = [this, &cycleN](const glm::ivec2& thresholds, size_t passes) {
        m_pushConstants.edgeConsolidationThresholds = thresholds;
        for (size_t i = 0; i < passes; i++) {
            m_pushConstants.edgeConsolidationCycle = cycleN++;
            m_commandBuffer->pushConstants<PushConstants>(m_consolidateEdgesPl.pipelineLayout(), eCompute, 0u, m_pushConstants);
            //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
            m_commandBuffer->dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
        }
    };
    auto doublePass = [pass](const glm::ivec2& firstThresholds, const glm::ivec2& secondThresholds, size_t passes) {
        for (size_t i = 0; i < passes; i++) {
            pass(firstThresholds, 1);
            pass(secondThresholds, 1);
        }
    };

    m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, *m_consolidateEdgesPl);
    doublePass({3, 4}, {4, 5}, 4);

    assert(static_cast<int>(cycleN) <= GEN_BORDER_WIDTH);
}

void ChunkGeneratorCS::selectVariant() {
    //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVariantPl);
    m_commandBuffer->pushConstants<PushConstants>(m_selectVariantPl.pipelineLayout(), eCompute, 0u, m_pushConstants);
    m_commandBuffer->dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
}

void ChunkGeneratorCS::finishGeneration(const RE::Texture& dstTex, const glm::ivec2& dstOffset) {
    m_commandBuffer->copyImage(
        m_tilesTex.image(), vk::ImageLayout::eTransferSrcOptimal,                                               //Src
        dstTex.image(), vk::ImageLayout::eTransferDstOptimal,                                                   //Dst
        vk::ImageCopy{
            vk::ImageSubresourceLayers{eColor, 0u, 0u, 1u}, vk::Offset3D{GEN_BORDER_WIDTH, GEN_BORDER_WIDTH, 0},//Src
            vk::ImageSubresourceLayers{eColor, 0u, 0u, 1u}, vk::Offset3D{dstOffset.x, dstOffset.y, 0},          //Dst
            vk::Extent3D{iCHUNK_SIZE.x, iCHUNK_SIZE.y, 1}                                                       //Extent
        }
    );
    m_commandBuffer->end();
    m_commandBuffer.submitToComputeQueue();
}
