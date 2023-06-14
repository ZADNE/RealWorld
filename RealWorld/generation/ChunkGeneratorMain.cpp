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
                  {0u, eStorageImage, 1u, eCompute},  // tilesImage
                  {1u, eStorageImage, 1u, eCompute},  // materialImage
                  {2u, eStorageBuffer, 1u, eCompute}, // LSystemSB
                  {3u, eStorageBuffer, 1u, eCompute}  // bodiesSB
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0u, sizeof(GenerationPC)}}}
      ) {
    m_descSet.write(
        vk::DescriptorType::eStorageImage, 0u, 0u, m_tilesTex, vk::ImageLayout::eGeneral
    );
    m_descSet.write(
        vk::DescriptorType::eStorageImage, 1u, 0u, m_materialTex, vk::ImageLayout::eGeneral
    );
    m_descSet.write(
        vk::DescriptorType::eStorageBuffer, 2u, 0u, m_lSystemBuf, 0ull, VK_WHOLE_SIZE
    );
}

void ChunkGenerator::setTarget(const TargetInfo& targetInfo) {
    m_genPC.seed     = targetInfo.seed;
    m_worldTex       = &targetInfo.worldTex;
    m_worldTexSizeCh = targetInfo.worldTexSizeCh;
    m_bodiesBuf      = &targetInfo.bodiesBuf;
    m_descSet.write(
        vk::DescriptorType::eStorageBuffer, 3u, 0u, *m_bodiesBuf, 0ull, VK_WHOLE_SIZE
    );
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
        S::eComputeShader,         // Src stage mask
        A::eShaderStorageWrite,    // Src access mask
        S::eComputeShader,         // Dst stage mask
        A::eShaderStorageRead,     // Dst access mask
        vk::ImageLayout::eGeneral, // Old image layout
        vk::ImageLayout::eGeneral, // New image layout
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, // Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0u, 1u, m_genPC.storeLayer, 1u}};
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
        vk::ImageLayout::eGeneral,                      // Old image layout
        vk::ImageLayout::eGeneral,                      // New image layout
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED, // Ownership transition
        m_tilesTex.image(),
        vk::ImageSubresourceRange{eColor, 0u, 1u, m_genPC.storeLayer, 1u}};
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    // Copy the generated chunk to the world texture
    auto dstOffsetTi = chToAt(posCh, m_worldTexSizeCh - 1);
    commandBuffer.copyImage(
        m_tilesTex.image(),
        vk::ImageLayout::eGeneral, // Src image
        m_worldTex->image(),
        vk::ImageLayout::eGeneral, // Dst image
        vk::ImageCopy{
            vk::ImageSubresourceLayers{eColor, 0u, m_genPC.storeLayer, 1u}, // Src subresource
            vk::Offset3D{k_genBorderWidth, k_genBorderWidth, 0}, // Src offset
            vk::ImageSubresourceLayers{eColor, 0u, 0u, 1u}, // Dst subresource
            vk::Offset3D{dstOffsetTi.x, dstOffsetTi.y, 0},  // Dst offset
            vk::Extent3D{iChunkTi.x, iChunkTi.y, 1}         // Copy Extent
        }
    );
}

} // namespace rw
