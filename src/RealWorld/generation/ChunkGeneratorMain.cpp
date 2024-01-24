/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/VegPreparationSB.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;
using B = vk::BufferUsageFlagBits;

namespace rw {

ChunkGenerator::ChunkGenerator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0, eStorageImage, 1, eCompute},  // Tiles image
                  {1, eStorageImage, 1, eCompute},  // Material image
                  {2, eUniformBuffer, 1, eCompute}, // VegTemplatesUB
                  {3, eStorageBuffer, 1, eCompute}, // Bodies
                  {4, eStorageBuffer, 1, eCompute}, // Vegetation buffer
                  {5, eStorageBuffer, 1, eCompute}, // Branch buffer
                  {6, eStorageBuffer, 1, eCompute}  // Veg Preparation
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0, sizeof(GenerationPC)}}}
      )
    , m_vegPreparationBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(VegPreparationSB),
          .usage       = B::eStorageBuffer | B::eIndirectBuffer,
          .debugName   = "rw::ChunkGenerator::vegPreparation"}) {
    m_descriptorSet.write(eStorageImage, 0, 0, m_tilesTex, eGeneral);
    m_descriptorSet.write(eStorageImage, 1, 0, m_materialTex, eGeneral);
    m_descriptorSet.write(eUniformBuffer, 2, 0, m_vegTemplatesBuf, 0, vk::WholeSize);
    m_descriptorSet.write(eStorageBuffer, 6, 0, m_vegPreparationBuf, 0, vk::WholeSize);
}

void ChunkGenerator::setTarget(const TargetInfo& targetInfo) {
    m_genPC.seed     = targetInfo.seed;
    m_worldTex       = &targetInfo.worldTex;
    m_worldTexSizeCh = targetInfo.worldTexSizeCh;
    m_bodiesBuf      = &targetInfo.bodiesBuf;
    m_vegBuf         = &targetInfo.vegBuf;
    m_branchBuf      = &targetInfo.branchBuf;
    m_descriptorSet.write(eStorageBuffer, 3, 0, *m_bodiesBuf, 0, vk::WholeSize);
    m_descriptorSet.write(eStorageBuffer, 4, 0, *m_vegBuf, 0, vk::WholeSize);
    m_descriptorSet.write(eStorageBuffer, 5, 0, *m_branchBuf, 0, vk::WholeSize);
}

void ChunkGenerator::nextStep() {
    m_nOfGenChunksThisStep = 0;
}

void ChunkGenerator::generateChunk(
    const re::CommandBuffer& cmdBuf, const OutputInfo& outputInfo
) {
    m_genPC.chunkOffsetTi  = chToTi(outputInfo.posCh);
    m_genPC.branchWriteBuf = outputInfo.branchWriteBuf;

    prepareToGenerate(cmdBuf);

    // Terrain generation
    generateBasicTerrain(cmdBuf);
    consolidateEdges(cmdBuf);
    selectVariant(cmdBuf);

    // Vegetation generation
    generateVegetation(cmdBuf);

    finishGeneration(cmdBuf, outputInfo.posCh);
}

void ChunkGenerator::finishGeneration(const re::CommandBuffer& cmdBuf, glm::ivec2 posCh) {
    // Wait for the generation to finish
    auto barriers = std::to_array(
        {re::imageMemoryBarrier(
             S::eComputeShader,                              // Src stage mask
             A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
             S::eTransfer,                                   // Dst stage mask
             A::eTransferRead,                               // Dst access mask
             eGeneral,                                       // Old image layout
             eGeneral,                                       // New image layout
             m_tilesTex.image(),
             vk::ImageSubresourceRange{eColor, 0, 1, m_genPC.storeLayer, 1}
         ),
         re::imageMemoryBarrier(
             S::eColorAttachmentOutput, // Src stage mask
             A::eColorAttachmentWrite,  // Src access mask
             S::eTransfer,              // Dst stage mask
             A::eTransferWrite,         // Dst access mask
             eGeneral,                  // Old image layout
             eGeneral,                  // New image layout
             m_worldTex->image()
         )}
    );
    cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, barriers});
    // Copy the generated chunk to the world texture
    auto dstOffsetTi = chToAt(posCh, m_worldTexSizeCh - 1);
    cmdBuf->copyImage(
        m_tilesTex.image(),
        eGeneral, // Src image
        m_worldTex->image(),
        eGeneral, // Dst image
        vk::ImageCopy{
            vk::ImageSubresourceLayers{eColor, 0, m_genPC.storeLayer, 1}, // Src subresource
            vk::Offset3D{k_genBorderWidth, k_genBorderWidth, 0}, // Src offset
            vk::ImageSubresourceLayers{eColor, 0, 0, 1},   // Dst subresource
            vk::Offset3D{dstOffsetTi.x, dstOffsetTi.y, 0}, // Dst offset
            vk::Extent3D{iChunkTi.x, iChunkTi.y, 1}        // Copy Extent
        }
    );
    m_nOfGenChunksThisStep++;
}

} // namespace rw
