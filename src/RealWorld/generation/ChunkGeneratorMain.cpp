/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/generation/ChunkGenerator.hpp>
#include <RealWorld/generation/VegPreparationSB.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;
using enum vk::ImageLayout;
using enum vk::ImageAspectFlagBits;
using enum vk::CommandBufferUsageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;
using B = vk::BufferUsageFlagBits;

namespace rw {

namespace {
using SB = VegPreparationSB;
struct VegPreparationSBInitHelper {
    decltype(SB::vegDispatchSize)    vegDispatchSize{0, 1, 1, 0};
    decltype(SB::branchDispatchSize) branchDispatchSize{0, 1, 1, 0};
};
using SBInitHelper = VegPreparationSBInitHelper;
static_assert(offsetof(SB, vegDispatchSize) == offsetof(SBInitHelper, vegDispatchSize));
static_assert(
    offsetof(SB, branchDispatchSize) == offsetof(SBInitHelper, branchDispatchSize)
);

constexpr VegPreparationSBInitHelper k_vegPreparationSBInitHelper{};
} // namespace

ChunkGenerator::ChunkGenerator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings = {{
                  {0, eStorageImage, 1, eCompute},      // Tiles image
                  {1, eStorageImage, 1, eCompute},      // Material image
                  {2, eUniformBuffer, 1, eCompute},     // VegTemplatesUB
                  {3, eStorageBuffer, 1, eCompute},     // Bodies
                  /*{4, eStorageBuffer, 1, eCompute},*/ // Reserved...
                  {5, eStorageBuffer, 1, eCompute},     // Branch buffer
                  {6, eStorageBuffer, 1, eCompute}      // Veg Preparation
              }},
              .ranges = {vk::PushConstantRange{eCompute, 0, sizeof(GenerationPC)}}}
      )
    , m_vegPreparationBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(VegPreparationSB),
          .usage     = B::eStorageBuffer | B::eIndirectBuffer | B::eTransferDst,
          .initData  = re::objectToByteSpan(k_vegPreparationSBInitHelper),
          .debugName = "rw::ChunkGenerator::vegPreparation"}) {
    m_descriptorSet.write(eStorageImage, 0, 0, m_tilesTex, eGeneral);
    m_descriptorSet.write(eStorageImage, 1, 0, m_materialTex, eGeneral);
    m_descriptorSet.write(eUniformBuffer, 2, 0, m_vegTemplatesBuf, 0, vk::WholeSize);
    m_descriptorSet.write(eStorageBuffer, 6, 0, m_vegPreparationBuf, 0, vk::WholeSize);
}

void ChunkGenerator::setTarget(const TargetInfo& targetInfo) {
    m_genPC.seed           = targetInfo.seed;
    m_worldTex             = &targetInfo.worldTex;
    m_genPC.worldTexSizeCh = targetInfo.worldTexSizeCh;
    m_bodiesBuf            = &targetInfo.bodiesBuf;
    m_branchBuf            = &targetInfo.branchBuf;
    m_descriptorSet.write(eStorageBuffer, 3, 0, *m_bodiesBuf, 0, vk::WholeSize);
    m_descriptorSet.write(eStorageBuffer, 5, 0, *m_branchBuf, 0, vk::WholeSize);
}

bool ChunkGenerator::planGeneration(glm::ivec2 posCh) {
    if (m_chunksPlanned < k_maxParallelChunks) { // If there is space
        m_genPC.chunkTi[m_chunksPlanned++] = chToTi(posCh);
        return true;
    }
    return false;
}

void ChunkGenerator::generate(const re::CommandBuffer& cmdBuf, glm::uint branchWriteBuf) {
    if (m_chunksPlanned > 0) {
        auto&                            secCmdBuf = *m_cmdBuf;
        vk::CommandBufferInheritanceInfo inheritanceInfo{};
        secCmdBuf->begin({eOneTimeSubmit, &inheritanceInfo});
        secCmdBuf->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descriptorSet, {}
        );
        m_genPC.branchWriteBuf = branchWriteBuf;

        // Terrain generation
        generateBasicTerrain(secCmdBuf);
        consolidateEdges(secCmdBuf);
        selectVariant(secCmdBuf);

        // Vegetation generation
        generateVegetation(secCmdBuf);

        copyToDestination(secCmdBuf);

        secCmdBuf->end({});
        cmdBuf->executeCommands(*secCmdBuf);
        m_chunksPlanned = 0;
    }
}

void ChunkGenerator::copyToDestination(const re::CommandBuffer& cmdBuf) {
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
             vk::ImageSubresourceRange{
                 eColor, 0, 1, k_maxParallelChunks * m_genPC.storeSegment, k_maxParallelChunks}
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
    std::array<vk::ImageCopy, k_maxParallelChunks> regions;
    for (int i = 0; i < m_chunksPlanned; ++i) {
        auto dstOffsetTi =
            chToAt(tiToCh(m_genPC.chunkTi[i]), m_genPC.worldTexSizeCh - 1);
        regions[i] = vk::ImageCopy{
            vk::ImageSubresourceLayers{
                eColor, 0, k_maxParallelChunks * m_genPC.storeSegment + i, 1}, // Src subresource
            vk::Offset3D{k_genBorderWidth, k_genBorderWidth, 0}, // Src offset
            vk::ImageSubresourceLayers{eColor, 0, 0, 1},   // Dst subresource
            vk::Offset3D{dstOffsetTi.x, dstOffsetTi.y, 0}, // Dst offset
            vk::Extent3D{iChunkTi.x, iChunkTi.y, 1}        // Copy Extent
        };
    }
    std::span spanOfRegions{regions.begin(), regions.begin() + m_chunksPlanned};
    cmdBuf->copyImage(
        m_tilesTex.image(),
        eGeneral, // Src image
        m_worldTex->image(),
        eGeneral, // Dst image
        spanOfRegions
    );
}

} // namespace rw
