/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGeneratorCS.hpp>

constexpr int GEN_CS_GROUP_SIZE = 16;
constexpr glm::uvec2 DISPATCH_SIZE = GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE;

using enum vk::ImageAspectFlagBits;
using enum vk::ShaderStageFlagBits;

ChunkGeneratorCS::ChunkGeneratorCS() {
    m_descSet.write(vk::DescriptorType::eStorageImage, 0u, 0u, m_tilesTex, vk::ImageLayout::eGeneral);
    m_descSet.write(vk::DescriptorType::eStorageImage, 1u, 0u, m_materialTex, vk::ImageLayout::eGeneral);
}

void ChunkGeneratorCS::prepareToGenerate() {
    m_commandBuffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    m_commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descSet, {});
}

void ChunkGeneratorCS::generateBasicTerrain() {
    m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateStructurePl);
    m_commandBuffer->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_pushConstants);
    m_commandBuffer->dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
    m_pushConstants.storeLayer = 1;
}

void ChunkGeneratorCS::consolidateEdges() {
    auto pass = [this](const glm::ivec2& thresholds, size_t passes) {
        m_pushConstants.edgeConsolidationPromote = thresholds.x;
        m_pushConstants.edgeConsolidationReduce = thresholds.y;
        for (size_t i = 0; i < passes; i++) {
            m_commandBuffer->pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_pushConstants);
            //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
            m_commandBuffer->dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
            m_pushConstants.storeLayer = ~m_pushConstants.storeLayer & 1;
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
}

void ChunkGeneratorCS::selectVariant() {
    //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVariantPl);
    m_commandBuffer->dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
}

void ChunkGeneratorCS::finishGeneration(const RE::Texture& dstTex, const glm::ivec2& dstOffset) {
    //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    m_commandBuffer->copyImage(
        m_tilesTex.image(), vk::ImageLayout::eGeneral,                                                          //Src
        dstTex.image(), vk::ImageLayout::eGeneral,                                                              //Dst
        vk::ImageCopy{
            {eColor, 0u, m_pushConstants.storeLayer, 1u},   vk::Offset3D{GEN_BORDER_WIDTH, GEN_BORDER_WIDTH, 0},//Src
            {eColor, 0u, 0u, 1u},                           vk::Offset3D{dstOffset.x, dstOffset.y, 0},          //Dst
            vk::Extent3D{iCHUNK_SIZE.x, iCHUNK_SIZE.y, 1}                                                       //Extent
        }
    );
    m_commandBuffer->end();
    m_commandBuffer.submitToComputeQueue();
}
