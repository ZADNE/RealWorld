/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGeneratorCS.hpp>

constexpr int GEN_CS_GROUP_SIZE = 16;
constexpr glm::uvec2 DISPATCH_SIZE = GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE;

using enum vk::ImageAspectFlagBits;
using enum vk::ShaderStageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

ChunkGeneratorCS::ChunkGeneratorCS() {
    m_descSet.write(vk::DescriptorType::eStorageImage, 0u, 0u, m_tilesTex, vk::ImageLayout::eGeneral);
    m_descSet.write(vk::DescriptorType::eStorageImage, 1u, 0u, m_materialTex, vk::ImageLayout::eGeneral);
}

void ChunkGeneratorCS::prepareToGenerate(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descSet, {});
}

void ChunkGeneratorCS::generateBasicTerrain(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_generateStructurePl);
    m_pushConstants.storeLayer = 0;
    commandBuffer.pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_pushConstants);
    commandBuffer.dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
}

void ChunkGeneratorCS::consolidateEdges(const vk::CommandBuffer& commandBuffer) {
    auto pass = [&](const glm::ivec2& thresholds, size_t passes) {
        m_pushConstants.edgeConsolidationPromote = thresholds.x;
        m_pushConstants.edgeConsolidationReduce = thresholds.y;
        for (size_t i = 0; i < passes; i++) {
            m_pushConstants.storeLayer = ~m_pushConstants.storeLayer & 1;
            //Wait for the previous pass to finish
            auto imageBarrier = stepBarrier();
            commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
            //Consolidate
            commandBuffer.pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_pushConstants);
            commandBuffer.dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
        }
    };
    auto doublePass = [pass](const glm::ivec2& firstThresholds, const glm::ivec2& secondThresholds, size_t passes) {
        for (size_t i = 0; i < passes; i++) {
            pass(firstThresholds, 1);
            pass(secondThresholds, 1);
        }
    };

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_consolidateEdgesPl);
    doublePass({3, 4}, {4, 5}, 4);
}

void ChunkGeneratorCS::selectVariant(const vk::CommandBuffer& commandBuffer) {
    //Wait for the edge consolidation to finish
    auto imageBarrier = stepBarrier();
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    //Select variants
    m_pushConstants.storeLayer = ~m_pushConstants.storeLayer & 1;
    commandBuffer.pushConstants<GenerationPC>(*m_pipelineLayout, eCompute, 0u, m_pushConstants);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_selectVariantPl);
    commandBuffer.dispatch(DISPATCH_SIZE.x, DISPATCH_SIZE.y, 1u);
}

void ChunkGeneratorCS::finishGeneration(const vk::CommandBuffer& commandBuffer, const RE::Texture& dstTex, const glm::ivec2& dstOffset) {
    //Wait for the generation to finish
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                                                          //Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead,                             //Src access mask
        S::eTransfer,                                                               //Dst stage mask
        A::eTransferRead,                                                           //Dst access mask
        vk::ImageLayout::eGeneral,                                                  //Old image layout
        vk::ImageLayout::eGeneral,                                                  //New image layout
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0u, 1u, m_pushConstants.storeLayer, 1u}
    };
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    //Copy the generated chunk to the world texture
    commandBuffer.copyImage(
        m_tilesTex.image(), vk::ImageLayout::eGeneral,                              //Src image
        dstTex.image(), vk::ImageLayout::eGeneral,                                  //Dst image
        vk::ImageCopy{
            vk::ImageSubresourceLayers{eColor, 0u, m_pushConstants.storeLayer, 1u}, //Src subresource
            vk::Offset3D{GEN_BORDER_WIDTH, GEN_BORDER_WIDTH, 0},                    //Src offset
            vk::ImageSubresourceLayers{eColor, 0u, 0u, 1u},                         //Dst subresource
            vk::Offset3D{dstOffset.x, dstOffset.y, 0},                              //Dst offset
            vk::Extent3D{iCHUNK_SIZE.x, iCHUNK_SIZE.y, 1}                           //Copy Extent
        }
    );
}

vk::ImageMemoryBarrier2 ChunkGeneratorCS::stepBarrier() const {
    return vk::ImageMemoryBarrier2{
        S::eComputeShader,                                                          //Src stage mask
        A::eShaderStorageWrite,                                                     //Src access mask
        S::eComputeShader,                                                          //Dst stage mask
        A::eShaderStorageRead,                                                      //Dst access mask
        vk::ImageLayout::eGeneral,                                                  //Old image layout
        vk::ImageLayout::eGeneral,                                                  //New image layout
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0u, 1u, m_pushConstants.storeLayer, 1u}
    };
}
