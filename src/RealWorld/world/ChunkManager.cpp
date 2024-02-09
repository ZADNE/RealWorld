/*!
 *  @author    Dubsky Tomas
 */
#include <future>

#include <RealEngine/graphics/synchronization/Fence.hpp>
#include <RealEngine/utility/Math.hpp>

#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/world/ActivationManager.hpp>
#include <RealWorld/world/ChunkManager.hpp>

using enum vk::ImageAspectFlagBits;
using enum vk::CommandBufferUsageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

ChunkManager::ChunkManager(ActivationManager& actManager)
    : m_actManager(actManager) {
}

void ChunkManager::setTarget(glm::ivec2 worldTexSizeCh) {
    m_worldTexSizeMask = worldTexSizeCh - 1;
    m_ts->resetTileStage();
}

bool ChunkManager::saveChunks(const re::Texture& worldTex) {
    // Save all active chunks (they have to be downloaded)
    assert(hasFreeTransferSpace());
    re::CommandBuffer cmdBuf{{.debugName = "rw::ChunkManager::saveChunks"}};
    re::Fence         downloadFinishedFence{{}};
    cmdBuf->begin({eOneTimeSubmit});

    { // Wait for unrasterization to finish
        auto imageBarrier = re::imageMemoryBarrier(
            S::eAllCommands,                 // Src stage mask
            A::eMemoryRead | A::eMemoryRead, // Src access mask
            S::eTransfer,                    // Dst stage mask
            A::eTransferRead,                // Dst access mask
            vk::ImageLayout::eGeneral,       // Old image layout
            vk::ImageLayout::eGeneral,       // New image layout
            worldTex.image()
        );
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    }

    auto saveAllChunksInTileStage = [&]() {
        std::array<std::future<void>, k_tileStageSlots> futures{};
        for (int i = k_tileStageSlots - 1; i > m_ts->nextDownloadSlot; --i) {
            futures[i] = std::async(std::launch::async, [this, i, &m_ts = m_ts]() {
                m_actManager.saveChunk(
                    &m_ts->buf[k_chunkByteSize * i], m_ts->targetCh[i]
                );
            });
        }
        m_ts->resetTileStage();
    };

    auto worldTexSize = m_worldTexSizeMask + 1;
    for (int y = 0; y < worldTexSize.y; ++y) {
        for (int x = 0; x < worldTexSize.x; ++x) {
            auto  posAc = glm::ivec2(x, y);
            auto& activeChunk =
                m_actManager.activeChunkAtIndex(acToIndex(posAc, worldTexSize));
            if (activeChunk != k_chunkNotActive) {
                if (hasFreeTransferSpace()) { // If there is space in the stage
                    planDownload(activeChunk, chToTi(posAc));
                } else {
                    endStep(cmdBuf, worldTex);
                    cmdBuf->end();
                    cmdBuf.submitToComputeQueue(*downloadFinishedFence);
                    downloadFinishedFence.wait();
                    downloadFinishedFence.reset();
                    saveAllChunksInTileStage();
                    cmdBuf->begin({eOneTimeSubmit});
                    planDownload(activeChunk, chToTi(posAc));
                }
            }
        }
    }

    endStep(cmdBuf, worldTex);

    { // Transition world texture back to original layout
        auto imageBarrier = re::imageMemoryBarrier(
            S::eTransfer,                      // Src stage mask
            A::eTransferRead,                  // Src access mask
            S::eAllCommands,                   // Dst stage mask
            {},                                // Dst access mask
            vk::ImageLayout::eGeneral,         // Old image layout
            vk::ImageLayout::eReadOnlyOptimal, // New image layout
            worldTex.image()
        );
        cmdBuf->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    }
    cmdBuf->end();
    cmdBuf.submitToComputeQueue(*downloadFinishedFence);
    downloadFinishedFence.wait();
    downloadFinishedFence.reset();
    saveAllChunksInTileStage();
    return true;
}

int ChunkManager::beginStep() {
    // Finalize uploads from previous step
    for (int i = 0; i < m_ts->nextUploadSlot; ++i) {
        glm::ivec2 posCh       = m_ts->targetCh[i];
        auto       posAc       = chToAc(posCh, m_worldTexSizeMask);
        auto&      activeChunk = m_actManager.activeChunkAtIndex(
            acToIndex(posAc, m_worldTexSizeMask + 1)
        );
        // Signal that the chunk is active
        activeChunk = posCh;
    }

    // Finalize downloads from previous step
    for (int i = k_tileStageSlots - 1; i > m_ts->nextDownloadSlot; --i) {
        glm::ivec2 posCh       = m_ts->targetCh[i];
        auto       posAc       = chToAc(posCh, m_worldTexSizeMask);
        auto&      activeChunk = m_actManager.activeChunkAtIndex(
            acToIndex(posAc, m_worldTexSizeMask + 1)
        );

        // Copy the tiles aside from the stage
        m_actManager.addInactiveChunk(
            posCh, Chunk{posCh, &m_ts->buf[k_chunkByteSize * i]}
        );

        // Signal that there is no active chunk at the spot
        activeChunk = k_chunkNotActive;
    }

    int nTransparentChanges = m_ts->nextUploadSlot;
    m_ts->resetTileStage();
    return nTransparentChanges;
}

bool ChunkManager::hasFreeTransferSpace() const {
    return m_ts->hasFreeTransferSpace();
}

void ChunkManager::planUpload(
    const std::vector<unsigned char>& tiles, glm::ivec2 posCh, glm::ivec2 posAt
) {
    assert(hasFreeTransferSpace());

    m_ts->targetCh[m_ts->nextUploadSlot] = posCh;
    auto bufOffset = static_cast<vk::DeviceSize>(m_ts->nextUploadSlot) *
                     k_chunkByteSize;
    std::memcpy(&m_ts->buf[bufOffset], tiles.data(), k_chunkByteSize);

    m_ts->copyRegions[m_ts->nextUploadSlot] = vk::BufferImageCopy2{
        bufOffset, // Buffer offset
        0u,
        0u,                          // Tightly packed
        {eColor, 0u, 0u, 1u},        // Subresource
        {posAt.x, posAt.y, 0u},      // Offset
        {iChunkTi.x, iChunkTi.y, 1u} // Extent
    };
    m_ts->nextUploadSlot++;
}

void ChunkManager::planDownload(glm::ivec2 posCh, glm::ivec2 posAt) {
    assert(hasFreeTransferSpace());

    m_ts->targetCh[m_ts->nextDownloadSlot] = posCh;
    auto bufOffset = static_cast<vk::DeviceSize>(m_ts->nextDownloadSlot) *
                     k_chunkByteSize;

    m_ts->copyRegions[m_ts->nextDownloadSlot] = vk::BufferImageCopy2{
        bufOffset, // Buffer offset
        0u,
        0u,                          // Tightly packed
        {eColor, 0u, 0u, 1u},        // Subresource
        {posAt.x, posAt.y, 0u},      // Offset
        {iChunkTi.x, iChunkTi.y, 1u} // Extent
    };
    m_ts->nextDownloadSlot--;
}

void ChunkManager::endStep(const re::CommandBuffer& cmdBuf, const re::Texture& worldTex) {
    // If there are uploads scheduled
    if (m_ts->nextUploadSlot > 0) {
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
            m_ts->copyRegions.begin(),
            m_ts->copyRegions.begin() + m_ts->nextUploadSlot};
        cmdBuf->copyBufferToImage2(vk::CopyBufferToImageInfo2{
            m_ts->buf.buffer(), worldTex.image(), vk::ImageLayout::eGeneral, spanOfRegions}
        );
    }

    // If there are downloads scheduled
    if (m_ts->nextDownloadSlot < k_tileStageSlots - 1) {
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
            m_ts->copyRegions.begin() + (m_ts->nextDownloadSlot + 1),
            m_ts->copyRegions.end()};
        cmdBuf->copyImageToBuffer2(vk::CopyImageToBufferInfo2{
            worldTex.image(), vk::ImageLayout::eGeneral, m_ts->buf.buffer(), spanOfRegions}
        );
    }
}

void ChunkManager::TileStage::resetTileStage() {
    nextUploadSlot   = 0;
    nextDownloadSlot = k_tileStageSlots - 1;
}

bool ChunkManager::TileStage::hasFreeTransferSpace() const {
    return nextUploadSlot <= nextDownloadSlot;
}

} // namespace rw
