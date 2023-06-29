/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

ChunkGenerator::ChunkGenerator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0, eStorageImage, 1, eCompute},  // tilesImage
                  {1, eStorageImage, 1, eCompute},  // materialImage
                  {2, eStorageBuffer, 1, eCompute}, // LSystemSB
                  {3, eStorageBuffer, 1, eCompute}, // bodiesSB
                  {4, eStorageBuffer, 1, eCompute}, // rootsSB
                  {5, eStorageBuffer, 1, eCompute}  // branchesSB
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0, sizeof(GenerationPC)}}}
      ) {
    m_descSet.write(eStorageImage, 0, 0, m_tilesTex, eGeneral);
    m_descSet.write(eStorageImage, 1, 0, m_materialTex, eGeneral);
    m_descSet.write(eStorageBuffer, 2, 0, m_lSystemBuf, 0, VK_WHOLE_SIZE);
}

void ChunkGenerator::setTarget(const TargetInfo& targetInfo) {
    m_genPC.seed     = targetInfo.seed;
    m_worldTex       = &targetInfo.worldTex;
    m_worldTexSizeCh = targetInfo.worldTexSizeCh;
    m_bodiesBuf      = &targetInfo.bodiesBuf;
    m_rootsBuf       = &targetInfo.rootsBuf;
    m_branchesBuf    = &targetInfo.branchesBuf;
    m_descSet.write(eStorageBuffer, 3, 0, *m_bodiesBuf, 0, VK_WHOLE_SIZE);
    m_descSet.write(eStorageBuffer, 4, 0, *m_rootsBuf, 0, VK_WHOLE_SIZE);
    m_descSet.write(eStorageBuffer, 5, 0, *m_branchesBuf, 0, VK_WHOLE_SIZE);
}

void ChunkGenerator::generateChunk(
    const vk::CommandBuffer& commandBuffer, const OutputInfo& outputInfo
) {
    m_genPC.chunkOffsetTi = chToTi(outputInfo.posCh);

    prepareToGenerate(commandBuffer);

    // Terrain generation
    generateBasicTerrain(commandBuffer);
    consolidateEdges(commandBuffer);
    selectVariant(commandBuffer);

    // Tree generation
    generateTrees(commandBuffer);

    finishGeneration(commandBuffer, outputInfo.posCh);
}

vk::ImageMemoryBarrier2 ChunkGenerator::stepBarrier() const {
    return vk::ImageMemoryBarrier2{
        S::eComputeShader,      // Src stage mask
        A::eShaderStorageWrite, // Src access mask
        S::eComputeShader,      // Dst stage mask
        A::eShaderStorageRead,  // Dst access mask
        eGeneral,               // Old image layout
        eGeneral,               // New image layout
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, // Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0, 1, m_genPC.storeLayer, 1}};
}

void ChunkGenerator::finishGeneration(
    const vk::CommandBuffer& commandBuffer, const glm::ivec2& posCh
) {
    // Wait for the generation to finish
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
        S::eTransfer,                                   // Dst stage mask
        A::eTransferRead,                               // Dst access mask
        eGeneral,                                       // Old image layout
        eGeneral,                                       // New image layout
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, // Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0, 1, m_genPC.storeLayer, 1}};
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    // Copy the generated chunk to the world texture
    auto dstOffsetTi = chToAt(posCh, m_worldTexSizeCh - 1);
    commandBuffer.copyImage(
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
