/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/constants/world.hpp>
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

constexpr glm::uint k_tilesImageBinding     = 0;
constexpr glm::uint k_materialImageBinding  = 1;
constexpr glm::uint k_vegTemplatesBinding   = 2;
constexpr glm::uint k_bodyBinding           = 3;
constexpr glm::uint k_branchBinding         = 4;
constexpr glm::uint k_branchAllocRegBinding = 5;
constexpr glm::uint k_vegPrepBinding        = 6;

} // namespace

ChunkGenerator::ChunkGenerator()
    : m_pipelineLayout(
          {},
          re::PipelineLayoutDescription{
              .bindings =
                  {{{k_tilesImageBinding, eStorageImage, 1, eCompute},
                    {k_materialImageBinding, eStorageImage, 1, eCompute},
                    {k_vegTemplatesBinding, eUniformBuffer, 1, eCompute},
                    {k_bodyBinding, eStorageBuffer, 1, eCompute},
                    {k_branchBinding, eStorageBuffer, 1, eCompute},
                    {k_branchAllocRegBinding, eStorageBuffer, 1, eCompute},
                    {k_vegPrepBinding, eStorageBuffer, 1, eCompute}}},
              .ranges = {vk::PushConstantRange{eCompute, 0, sizeof(GenerationPC)}}}
      )
    , m_vegPreparationBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(VegPreparationSB),
          .usage     = B::eStorageBuffer | B::eIndirectBuffer | B::eTransferDst,
          .initData  = re::objectToByteSpan(k_vegPreparationSBInitHelper),
          .debugName = "rw::ChunkGenerator::vegPreparation"}) {
    m_descriptorSet.write(eStorageImage, k_tilesImageBinding, 0, m_tilesTex, eGeneral);
    m_descriptorSet.write(
        eStorageImage, k_materialImageBinding, 0, m_materialTex, eGeneral
    );
    m_descriptorSet.write(eUniformBuffer, k_vegTemplatesBinding, 0, m_vegTemplatesBuf);
    m_descriptorSet.write(eStorageBuffer, k_vegPrepBinding, 0, m_vegPreparationBuf);
}

void ChunkGenerator::setTarget(const TargetInfo& targetInfo) {
    m_genPC.seed           = targetInfo.seed;
    m_worldTex             = &targetInfo.worldTex;
    m_genPC.worldTexSizeCh = targetInfo.worldTexSizeCh;
    m_bodiesBuf            = &targetInfo.bodiesBuf;
    m_branchBuf            = &targetInfo.branchBuf;
    m_descriptorSet.write(eStorageBuffer, k_bodyBinding, 0, *m_bodiesBuf);
    m_descriptorSet.write(eStorageBuffer, k_branchBinding, 0, *m_branchBuf);
    m_descriptorSet.write(
        eStorageBuffer, k_branchAllocRegBinding, 0, targetInfo.branchAllocRegBuf
    );
}

bool ChunkGenerator::planGeneration(glm::ivec2 posCh) {
    if (m_chunksPlanned < k_chunkGenSlots) { // If there is space
        m_genPC.chunkTi[m_chunksPlanned++] = chToTi(posCh);
        return true;
    }
    return false;
}

bool ChunkGenerator::generate(const ActionCmdBuf& acb) {
    if (m_chunksPlanned > 0) {
        auto&                            secCb = *m_cb;
        vk::CommandBufferInheritanceInfo inheritanceInfo{};
        secCb->begin({eOneTimeSubmit, &inheritanceInfo});
        secCb->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descriptorSet, {}
        );

        // Terrain generation
        generateBasicTerrain(secCb);
        consolidateEdges(secCb);
        selectVariant(secCb);

        // Vegetation generation
        generateVegetation(secCb);

        secCb->end({});
        (*acb)->executeCommands(*secCb);

        copyToDestination(acb);

        m_chunksPlanned = 0;
        return true;
    }
    return false;
}

void ChunkGenerator::copyToDestination(const ActionCmdBuf& acb) {
    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Wait for the generation to finish (untracked in action accesses)
            auto barriers = std::to_array({re::imageMemoryBarrier(
                S::eComputeShader, // Src stage mask
                A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
                S::eTransfer,     // Dst stage mask
                A::eTransferRead, // Dst access mask
                eGeneral,         // Old image layout
                eGeneral,         // New image layout
                m_tilesTex.image(),
                vk::ImageSubresourceRange{
                    eColor, 0, 1, k_chunkGenSlots * m_genPC.storeSegment, k_chunkGenSlots}
            )});
            cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, barriers});

            // Copy the generated chunk to the world texture
            std::array<vk::ImageCopy, k_chunkGenSlots> regions;
            for (int i = 0; i < m_chunksPlanned; ++i) {
                auto dstOffsetTi =
                    chToAt(tiToCh(m_genPC.chunkTi[i]), m_genPC.worldTexSizeCh - 1);
                regions[i] = vk::ImageCopy{
                    vk::ImageSubresourceLayers{
                        eColor, 0, k_chunkGenSlots * m_genPC.storeSegment + i, 1}, // Src subresource
                    vk::Offset3D{k_genBorderWidth, k_genBorderWidth, 0}, // Src offset
                    vk::ImageSubresourceLayers{eColor, 0, 0, 1}, // Dst subresource
                    vk::Offset3D{dstOffsetTi.x, dstOffsetTi.y, 0}, // Dst offset
                    vk::Extent3D{iChunkTi.x, iChunkTi.y, 1} // Copy Extent
                };
            }
            std::span spanOfRegions{regions.begin(), regions.begin() + m_chunksPlanned};
            cb->copyImage(
                m_tilesTex.image(),
                eGeneral, // Src image
                m_worldTex->image(),
                eGeneral, // Dst image
                spanOfRegions
            );
        },
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eTransfer,
            .access = A::eTransferWrite}
    );
}

} // namespace rw
