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
using B = vk::BufferUsageFlagBits;

namespace rw {

struct TreeDescription {
    glm::uint templateRootIndex; // Index to the template buffer
    glm::uint writeIndex;        // Index to the branch buffer
    glm::uint randomSeed;
    glm::uint padding;
    glm::vec2 rootPosTi;
    float     sizeFactor;
    float     angleFactor;
};

ChunkGenerator::ChunkGenerator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0, eStorageImage, 1, eCompute},  // tilesImage
                  {1, eStorageImage, 1, eCompute},  // materialImage
                  {2, eUniformBuffer, 1, eCompute}, // TreeTemplatesUB
                  {3, eStorageBuffer, 1, eCompute}, // bodiesSB
                  {4, eStorageBuffer, 1, eCompute}, // branchesSBWrite
                  {5, eStorageBuffer, 1, eCompute}, // branchesSBWrite
                  {6, eStorageBuffer, 1, eCompute}  // TreePreparationSB
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0, sizeof(GenerationPC)}}}
      )
    , m_treePreparationBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(glm::uvec4) + sizeof(TreeDescription) * 32,
          .usage       = B::eStorageBuffer | B::eIndirectBuffer}) {
    m_descSet.forEach([&](auto& ds) {
        ds.write(eStorageImage, 0, 0, m_tilesTex, eGeneral);
        ds.write(eStorageImage, 1, 0, m_materialTex, eGeneral);
        ds.write(eUniformBuffer, 2, 0, m_treeTemplatesBuf, 0, vk::WholeSize);
        ds.write(eStorageBuffer, 6, 0, m_treePreparationBuf, 0, vk::WholeSize);
    });
}

void ChunkGenerator::setTarget(const TargetInfo& targetInfo) {
    m_genPC.seed     = targetInfo.seed;
    m_worldTex       = &targetInfo.worldTex;
    m_worldTexSizeCh = targetInfo.worldTexSizeCh;
    m_bodiesBuf      = &targetInfo.bodiesBuf;
    m_branchesBuf.forEach(
        [&](auto& buf, const auto& branchBuf) { buf = &branchBuf; },
        targetInfo.branchesBuf
    );
    m_descSet.forEach(
        [&](auto& ds, const auto& branchBuf) {
            ds.write(eStorageBuffer, 3, 0, *m_bodiesBuf, 0, vk::WholeSize);
        },
        m_branchesBuf
    );
    auto writeDescriptor = [&](re::DescriptorSet& set,
                               const re::Buffer&  first,
                               const re::Buffer&  second) {
        set.write(eStorageBuffer, 4, 0, first, 0, vk::WholeSize);
        set.write(eStorageBuffer, 5, 0, second, 0, vk::WholeSize);
    };
    writeDescriptor(m_descSet[0], *m_branchesBuf[0], *m_branchesBuf[1]);
    writeDescriptor(m_descSet[1], *m_branchesBuf[1], *m_branchesBuf[0]);
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
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored, // Ownership transition
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
