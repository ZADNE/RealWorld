/*!
 *  @author    Dubsky Tomas
 */
#include <future>

#include <RealEngine/graphics/synchronization/Fence.hpp>
#include <RealEngine/utility/Math.hpp>

#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/world/ChunkActivationMgr.hpp>
#include <RealWorld/world/ChunkTransferMgr.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>

using enum vk::ImageAspectFlagBits;
using enum vk::CommandBufferUsageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

ChunkTransferMgr::ChunkTransferMgr(const re::PipelineLayout& pipelineLayout)
    : m_saveVegetationPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::ChunkTransferMgr::saveVegetation"},
          {.comp = saveVegetation_comp}
      ) {
}

void ChunkTransferMgr::reset() {
    m_stage.forEach([](auto& ts) { ts.reset(); });
}

bool ChunkTransferMgr::saveChunks(
    const re::Texture&  worldTex,
    const re::Buffer&   branchBuf,
    glm::ivec2          worldTexCh,
    ChunkActivationMgr& actMgr
) {
    // Save all active chunks (they have to be downloaded)
    re::CommandBuffer cmdBuf{{.debugName = "rw::ChunkTransferMgr::saveChunks"}};
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
        std::array<std::future<void>, k_stageSlotCount> futures{};
        for (int i = m_stage->nextDownloadSlot + 1; i < k_stageSlotCount; ++i) {
            futures[i] = std::async(
                std::launch::async,
                [this, i, &m_stage = m_stage, &actMgr]() {
                    actMgr.saveChunk(
                        m_stage->slots[i].targetCh, m_stage->buf->tiles[i].data(), {}
                    );
                }
            );
        }
        m_stage->reset();
    };

    for (int y = 0; y < worldTexCh.y; ++y) {
        for (int x = 0; x < worldTexCh.x; ++x) {
            auto  posAc = glm::ivec2(x, y);
            auto& activeChunk =
                actMgr.activeChunkAtIndex(acToIndex(posAc, worldTexCh));
            if (activeChunk != k_chunkNotActive) {
                if (hasFreeTransferSpace(posAc)) { // If there is space in the stage
                    planDownload(activeChunk, chToTi(posAc), 0);
                } else {
                    endStep(cmdBuf, worldTex, branchBuf);
                    cmdBuf->end();
                    cmdBuf.submitToComputeQueue(*downloadFinishedFence);
                    downloadFinishedFence.wait();
                    downloadFinishedFence.reset();
                    saveAllChunksInTileStage();
                    cmdBuf->begin({eOneTimeSubmit});
                    planDownload(activeChunk, chToTi(posAc), 0);
                }
            }
        }
    }

    endStep(cmdBuf, worldTex, branchBuf);

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

int ChunkTransferMgr::beginStep(glm::ivec2 worldTexCh, ChunkActivationMgr& actMgr) {
    glm::ivec2 worldTexSizeMask = worldTexCh - 1;
    // Finalize uploads from previous step
    for (int i = 0; i < m_stage->nextUploadSlot; ++i) {
        auto slot  = m_stage->slots[i];
        auto posAc = chToAc(slot.targetCh, worldTexSizeMask);
        auto& activeChunk = actMgr.activeChunkAtIndex(acToIndex(posAc, worldTexCh));
        // Signal that the chunk is active
        activeChunk = slot.targetCh;
    }

    // Finalize downloads from previous step
    const auto& stage                  = *m_stage->buf;
    int         nextBranchDownloadByte = m_stage->nextBranchDownloadByte;
    for (int i = m_stage->nextDownloadSlot + 1; i < k_stageSlotCount; ++i) {
        auto slot  = m_stage->slots[i];
        auto posAc = chToAc(slot.targetCh, worldTexSizeMask);
        auto& activeChunk = actMgr.activeChunkAtIndex(acToIndex(posAc, worldTexCh));

        // Copy the tiles aside from the stage
        auto branchesSerialized = std::span{
            &stage.branches[nextBranchDownloadByte],
            &stage.branches[nextBranchDownloadByte + slot.branchBytes]};
        actMgr.addInactiveChunk(
            slot.targetCh,
            Chunk{slot.targetCh, stage.tiles[i].data(), branchesSerialized}
        );
        nextBranchDownloadByte += slot.branchBytes;

        // Signal that there is no active chunk at the spot
        activeChunk = k_chunkNotActive;
    }

    int nTransparentChanges = m_stage->nextUploadSlot;
    m_stage->reset();
    return nTransparentChanges;
}

bool ChunkTransferMgr::hasFreeTransferSpace(glm::ivec2 posAc) const {
    return m_stage->hasFreeTransferSpace(branchCount(posAc));
}

void ChunkTransferMgr::planUpload(
    glm::ivec2                  posCh,
    glm::ivec2                  posAt,
    const std::vector<uint8_t>& tiles,
    std::span<const uint8_t>    branchesSerialized
) {
    m_stage->slots[m_stage->nextUploadSlot].targetCh = posCh;
    auto bufOffset = static_cast<vk::DeviceSize>(m_stage->nextUploadSlot) *
                     k_chunkByteSize;
    std::memcpy(&m_stage->buf[bufOffset], tiles.data(), k_chunkByteSize);

    m_stage->tileCopyRegions[m_stage->nextUploadSlot] = vk::BufferImageCopy2{
        bufOffset, // Buffer offset
        0u,
        0u,                          // Tightly packed
        {eColor, 0u, 0u, 1u},        // Subresource
        {posAt.x, posAt.y, 0u},      // Offset
        {iChunkTi.x, iChunkTi.y, 1u} // Extent
    };
    m_stage->nextUploadSlot++;
}

void ChunkTransferMgr::planDownload(
    glm::ivec2 posCh, glm::ivec2 posAt, glm::uint branchReadBuf
) {
    { // Plan download of tiles
        auto bufOffset = static_cast<vk::DeviceSize>(m_stage->nextDownloadSlot) *
                         k_chunkByteSize;

        m_stage->tileCopyRegions[m_stage->nextDownloadSlot] = vk::BufferImageCopy2{
            bufOffset, // Buffer offset
            0u,
            0u,                          // Tightly packed
            {eColor, 0u, 0u, 1u},        // Subresource
            {posAt.x, posAt.y, 0u},      // Offset
            {iChunkTi.x, iChunkTi.y, 1u} // Extent
        };
    }
    // Plan download of branches
    int allocI      = allocIndex(tiToCh(posAt));
    int branchCount = 0;
    if (allocI >= 0) {
        const auto& alloc = m_regBuf->allocations[allocI];
        auto        i     = alloc.firstBranch;
        branchCount       = alloc.branchCount;
        assert(branchCount > 0);
        vk::DeviceSize sizeBytes = sizeof(BranchSerialized::absPosTi) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, absPosTi[branchReadBuf][i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::absAngNorm) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, absAngNorm[branchReadBuf][i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::parentOffset15wallType31) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, parentOffset15wallType31[i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::relRestAngNorm) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, relRestAngNorm[i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::angVel) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, angVel[i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::radiusTi) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, radiusTi[i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::lengthTi) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, lengthTi[i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::densityStiffness) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, densityStiffness[i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
        sizeBytes = sizeof(BranchSerialized::raster) * branchCount;
        m_stage->nextBranchDownloadByte -= sizeBytes;
        m_stage->branchCopyRegions[--m_stage->nextBranchDownloadSlot] = vk::BufferCopy2{
            offsetof(BranchSB, raster[i]),
            offsetof(StageBuf, branches) + m_stage->nextBranchDownloadByte,
            sizeBytes};
    }
    m_stage->slots[m_stage->nextDownloadSlot] =
        Stage::Slot{posCh, branchCount * sizeof(BranchSerialized)};
    m_stage->nextDownloadSlot--;
}

void ChunkTransferMgr::endStep(
    const re::CommandBuffer& cmdBuf,
    const re::Texture&       worldTex,
    const re::Buffer&        branchBuf
) {
    // If there are uploads scheduled
    if (m_stage->nextUploadSlot > 0) {
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
            m_stage->tileCopyRegions.begin(),
            m_stage->tileCopyRegions.begin() + m_stage->nextUploadSlot};
        cmdBuf->copyBufferToImage2(vk::CopyBufferToImageInfo2{
            m_stage->buf.buffer(),
            worldTex.image(),
            vk::ImageLayout::eGeneral,
            spanOfRegions});
    }

    // If there are downloads scheduled
    if (m_stage->nextDownloadSlot < k_stageSlotCount - 1) {
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

        { // Copy tiles
            std::span spanOfRegions{
                m_stage->tileCopyRegions.begin() + (m_stage->nextDownloadSlot + 1),
                m_stage->tileCopyRegions.end()};
            cmdBuf->copyImageToBuffer2(vk::CopyImageToBufferInfo2{
                worldTex.image(),
                vk::ImageLayout::eGeneral,
                m_stage->buf.buffer(),
                spanOfRegions});
        }

        { // Copy branches
            std::span spanOfRegions{
                m_stage->branchCopyRegions.begin() + m_stage->nextBranchDownloadSlot,
                m_stage->branchCopyRegions.end()};
            cmdBuf->copyBuffer2(vk::CopyBufferInfo2{
                *branchBuf, m_stage->buf.buffer(), spanOfRegions});
        }
    }
}

void ChunkTransferMgr::downloadBranchAllocRegister(
    const re::CommandBuffer& cmdBuf, const re::Buffer& branchBuf
) {
    vk::BufferCopy2 copyRegion{
        offsetof(BranchSB, allocReg), 0ull, sizeof(BranchAllocRegister)};
    cmdBuf->copyBuffer2({*branchBuf, m_regBuf.buffer(), copyRegion});
}

int ChunkTransferMgr::allocIndex(glm::ivec2 posAc) const {
    return m_regBuf
        ->allocIndexOfTheChunk[posAc.y * k_maxWorldTexSizeCh.x + posAc.x];
}

int ChunkTransferMgr::branchCount(glm::ivec2 posAc) const {
    if (int allocI = allocIndex(posAc); allocI >= 0) {
        return m_regBuf->allocations[allocI].branchCount;
    }
    return 0;
}

void ChunkTransferMgr::Stage::reset() {
    nextUploadSlot         = 0;
    nextDownloadSlot       = k_stageSlotCount - 1;
    nextBranchUploadSlot   = 0;
    nextBranchDownloadSlot = branchCopyRegions.size();
    nextBranchUploadByte   = 0;
    nextBranchDownloadByte = sizeof(StageBuf::branches);
}

bool ChunkTransferMgr::Stage::hasFreeTransferSpace(int branchCount) const {
    bool slotsAvailable       = nextUploadSlot <= nextDownloadSlot;
    bool branchBytesAvailable = nextBranchUploadByte +
                                    (branchCount * sizeof(BranchSerialized)) <=
                                nextBranchDownloadByte;
    return slotsAvailable && branchBytesAvailable;
}

} // namespace rw
