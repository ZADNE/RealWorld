/*!
 *  @author    Dubsky Tomas
 */
#include <future>

#include <RealEngine/graphics/synchronization/Fence.hpp>
#include <RealEngine/utility/Math.hpp>

#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/world/ActivationManager.hpp>
#include <RealWorld/world/ChunkManager.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::ImageAspectFlagBits;
using enum vk::CommandBufferUsageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

ChunkManager::ChunkManager(
    const re::PipelineLayout& pipelineLayout, ActivationManager& actManager
)
    : m_tilesStageBuf(re::BufferCreateInfo{
          .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                        vma::AllocationCreateFlagBits::eHostAccessRandom,
          .sizeInBytes = k_tileStageSize * k_chunkByteSize,
          .usage       = eTransferSrc | eTransferDst,
          .debugName   = "rw::ChunkManager::tilesStage"})
    , m_actManager(actManager) {
}

void ChunkManager::setTarget(glm::ivec2 worldTexSizeCh) {
    m_worldTexSizeMask = worldTexSizeCh - 1;
    resetTileStages();
}

bool ChunkManager::saveChunks(const re::Texture& worldTex) {
    // Save all active chunks (they have to be downloaded)
    assert(hasFreeTransferSpace());
    re::CommandBuffer cmdBuf{{.debugName = "rw::ChunkManager::saveChunks"}};
    re::Fence         downloadFinishedFence{{}};
    cmdBuf->begin({eOneTimeSubmit});

    auto imageBarrier = re::imageMemoryBarrier(
        S::eAllCommands,           // Src stage mask
        {},                        // Src access mask
        S::eTransfer,              // Dst stage mask
        A::eTransferRead,          // Dst access mask
        vk::ImageLayout::eGeneral, // Old image layout
        vk::ImageLayout::eGeneral, // New image layout
        worldTex.image()
    );
    cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

    auto saveAllChunksInTileStage = [&]() {
        std::array<std::future<void>, k_tileStageSize> futures{};
        for (size_t i = k_tileStageSize - 1; i > m_nextDownloadTileStage; --i) {
            futures[i] = std::async(std::launch::async, [this, i]() {
                m_actManager.saveChunk(
                    &m_tilesStageBuf[k_chunkByteSize * i], m_tileStageTargetCh[i]
                );
            });
        }
        resetTileStages();
    };

    auto worldTexSize = m_worldTexSizeMask + 1;
    for (int y = 0; y < worldTexSize.y; ++y) {
        for (int x = 0; x < worldTexSize.x; ++x) {
            auto  posAc = glm::ivec2(x, y);
            auto& activeChunk =
                m_actManager.activeChunkAtIndex(acToIndex(posAc, worldTexSize));
            if (activeChunk != k_chunkNotActive) {
                if (hasFreeTransferSpace()) { // If there is space in the stage
                    planDownload(cmdBuf, activeChunk, chToTi(posAc));
                } else {
                    cmdBuf->end();
                    cmdBuf.submitToComputeQueue(*downloadFinishedFence);
                    downloadFinishedFence.wait();
                    downloadFinishedFence.reset();
                    saveAllChunksInTileStage();
                    cmdBuf->begin({eOneTimeSubmit});
                }
            }
        }
    }

    imageBarrier = re::imageMemoryBarrier(
        S::eTransfer,                      // Src stage mask
        A::eTransferRead,                  // Src access mask
        S::eAllCommands,                   // Dst stage mask
        {},                                // Dst access mask
        vk::ImageLayout::eGeneral,         // Old image layout
        vk::ImageLayout::eReadOnlyOptimal, // New image layout
        worldTex.image()
    );
    cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

    cmdBuf->end();
    cmdBuf.submitToComputeQueue(*downloadFinishedFence);
    downloadFinishedFence.wait();
    downloadFinishedFence.reset();
    saveAllChunksInTileStage();
    return true;
}

int ChunkManager::beginStep() {
    // Finalize uploads from previous step
    for (size_t i = 0; i < m_nextUploadTileStage; ++i) {
        glm::ivec2 posCh       = m_tileStageTargetCh[i];
        auto       posAc       = chToAc(posCh, m_worldTexSizeMask);
        auto&      activeChunk = m_actManager.activeChunkAtIndex(
            acToIndex(posAc, m_worldTexSizeMask + 1)
        );
        // Signal that the chunk is active
        activeChunk = posCh;
    }

    // Finalize downloads from previous step
    for (size_t i = k_tileStageSize - 1; i > m_nextDownloadTileStage; --i) {
        glm::ivec2 posCh       = m_tileStageTargetCh[i];
        auto       posAc       = chToAc(posCh, m_worldTexSizeMask);
        auto&      activeChunk = m_actManager.activeChunkAtIndex(
            acToIndex(posAc, m_worldTexSizeMask + 1)
        );

        // Copy the tiles aside from the stage
        m_actManager.addInactiveChunk(
            posCh, Chunk{posCh, &m_tilesStageBuf[k_chunkByteSize * i]}
        );

        // Signal that there is no active chunk at the spot
        activeChunk = k_chunkNotActive;
    }

    int nTransparentChanges = m_nextUploadTileStage;
    resetTileStages();
    return nTransparentChanges;
}

bool ChunkManager::hasFreeTransferSpace() const {
    return m_nextUploadTileStage < m_nextDownloadTileStage;
}

void ChunkManager::planUpload(
    const std::vector<unsigned char>& tiles, glm::ivec2 posCh, glm::ivec2 posAt
) {
    assert(hasFreeTransferSpace());

    m_tileStageTargetCh[m_nextUploadTileStage] = posCh;
    auto bufOffset = static_cast<vk::DeviceSize>(m_nextUploadTileStage) *
                     k_chunkByteSize;
    std::memcpy(&m_tilesStageBuf[bufOffset], tiles.data(), k_chunkByteSize);

    m_copyRegions[m_nextUploadTileStage] = vk::BufferImageCopy2{
        bufOffset, // Buffer offset
        0u,
        0u,                          // Tightly packed
        {eColor, 0u, 0u, 1u},        // Subresource
        {posAt.x, posAt.y, 0u},      // Offset
        {iChunkTi.x, iChunkTi.y, 1u} // Extent
    };
    m_nextUploadTileStage++;
}

void ChunkManager::planDownload(
    const re::CommandBuffer& cmdBuf, glm::ivec2 posCh, glm::ivec2 posAt
) {
    assert(hasFreeTransferSpace());

    m_tileStageTargetCh[m_nextDownloadTileStage] = posCh;
    auto bufOffset = static_cast<vk::DeviceSize>(m_nextDownloadTileStage) *
                     k_chunkByteSize;

    m_copyRegions[m_nextDownloadTileStage] = vk::BufferImageCopy2{
        bufOffset, // Buffer offset
        0u,
        0u,                          // Tightly packed
        {eColor, 0u, 0u, 1u},        // Subresource
        {posAt.x, posAt.y, 0u},      // Offset
        {iChunkTi.x, iChunkTi.y, 1u} // Extent
    };
    m_nextDownloadTileStage--;
}

void ChunkManager::endStep(const re::CommandBuffer& cmdBuf, const re::Texture& worldTex) {
    // If there are uploads scheduled
    if (m_nextUploadTileStage > 0) {
        // Wait for unrasterization to finish
        auto imageBarrier = re::imageMemoryBarrier(
            S::eColorAttachmentOutput | S::eTransfer, // Src stage mask
            A::eColorAttachmentRead | A::eColorAttachmentWrite |
                A::eTransferRead | A::eTransferWrite, // Src access mask
            S::eTransfer,                             // Dst stage mask
            A::eTransferWrite,                        // Dst access mask
            vk::ImageLayout::eGeneral,                // Old image layout
            vk::ImageLayout::eGeneral,                // New image layout
            worldTex.image()
        );
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

        std::span spanOfRegions{
            m_copyRegions.begin(), m_copyRegions.begin() + m_nextUploadTileStage};
        cmdBuf->copyBufferToImage2(vk::CopyBufferToImageInfo2{
            m_tilesStageBuf.buffer(),
            worldTex.image(),
            vk::ImageLayout::eGeneral,
            spanOfRegions});
    }

    // If there are downloads scheduled
    if (m_nextDownloadTileStage < k_tileStageSize - 1) {
        // Wait for unrasterization to finish
        auto imageBarrier = re::imageMemoryBarrier(
            S::eColorAttachmentOutput | S::eTransfer, // Src stage mask
            A::eColorAttachmentRead | A::eColorAttachmentWrite |
                A::eTransferRead | A::eTransferWrite, // Src access mask
            S::eTransfer,                             // Dst stage mask
            A::eTransferRead,                         // Dst access mask
            vk::ImageLayout::eGeneral,                // Old image layout
            vk::ImageLayout::eGeneral,                // New image layout
            worldTex.image()
        );
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

        std::span spanOfRegions{
            m_copyRegions.begin() + m_nextDownloadTileStage + 1,
            m_copyRegions.end()};
        cmdBuf->copyImageToBuffer2(vk::CopyImageToBufferInfo2{
            worldTex.image(),
            vk::ImageLayout::eGeneral,
            m_tilesStageBuf.buffer(),
            spanOfRegions});
    }
}

void ChunkManager::resetTileStages() {
    m_nextUploadTileStage   = 0;
    m_nextDownloadTileStage = k_tileStageSize - 1;
}

} // namespace rw
