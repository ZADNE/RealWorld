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

void ChunkGenerator::generateChunk(
    const vk::CommandBuffer& cmdBuf, const OutputInfo& outputInfo
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

vk::ImageMemoryBarrier2 ChunkGenerator::worldTexBarrier() const {
    return vk::ImageMemoryBarrier2{
        S::eComputeShader,      // Src stage mask
        A::eShaderStorageWrite, // Src access mask
        S::eComputeShader,      // Dst stage mask
        A::eShaderStorageRead,  // Dst access mask
        eGeneral,               // Old image layout
        eGeneral,               // New image layout
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored, // Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0, 1, m_genPC.storeLayer, 1}};
}

void ChunkGenerator::finishGeneration(const vk::CommandBuffer& cmdBuf, glm::ivec2 posCh) {
    // Wait for the generation to finish
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
        S::eTransfer,                                   // Dst stage mask
        A::eTransferRead,                               // Dst access mask
        eGeneral,                                       // Old image layout
        eGeneral,                                       // New image layout
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored, // Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0, 1, m_genPC.storeLayer, 1}};
    cmdBuf.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    // Copy the generated chunk to the world texture
    auto dstOffsetTi = chToAt(posCh, m_worldTexSizeCh - 1);
    cmdBuf.copyImage(
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
}

} // namespace rw
