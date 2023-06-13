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
                  {0u, eStorageImage, 1u, eCompute}, // tilesImage
                  {1u, eStorageImage, 1u, eCompute}, // materialImage
                  {2u, eStorageBuffer, 1u, eCompute} // LSystemSB
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

void ChunkGenerator::setSeed(int seed) {
    m_genPC.seed = seed;
}

void ChunkGenerator::generateChunk(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2&        posCh,
    const OutputInfo&        outputInfo
) {
    m_genPC.chunkOffsetTi = posCh * iChunkTi;

    prepareToGenerate(commandBuffer);

    // Terrain generation
    generateBasicTerrain(commandBuffer);
    consolidateEdges(commandBuffer);
    selectVariant(commandBuffer);

    // Tree generation
    generateTrees(commandBuffer);

    finishGeneration(commandBuffer, outputInfo.dstTex, outputInfo.dstOffsetTi);
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

} // namespace rw
