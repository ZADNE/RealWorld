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

using enum vk::CommandBufferUsageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

ChunkTransferMgr::ChunkTransferMgr(const re::PipelineLayout& pipelineLayout)
    : m_allocBranchesPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::ChunkTransferMgr::allocBranches"},
          {.comp = allocBranches_comp}
      ) {
}

void ChunkTransferMgr::setTarget(glm::ivec2 worldTexCh) {
    m_worldTexCh = worldTexCh;
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

    auto saveAllChunksInTileStage = [&] {
        std::array<std::future<void>, k_stageSlotCount> futures{};
        m_stage->forEachDownload([&](int i) {
            futures[i] = std::async(
                std::launch::async,
                [this, i, &stage = m_stage, &actMgr]() {
                    auto slt = stage->slot(i);
                    actMgr.saveChunk(
                        slt.targetCh,
                        stage->tiles(i),
                        stage->branchesSerializedSpan(slt)
                    );
                }
            );
        });
        m_stage->reset();
    };

    auto recordAllPlannedDownload = [&] {
        // Download tiles
        cmdBuf->copyImageToBuffer2(vk::CopyImageToBufferInfo2{
            worldTex.image(),
            vk::ImageLayout::eGeneral,
            *m_stage->buffer(),
            m_stage->tileDownloadRegions()});

        // Download branches
        if (m_stage->numberOfBranchDownloads()) {
            cmdBuf->copyBuffer2(vk::CopyBufferInfo2{
                *branchBuf, *m_stage->buffer(), m_stage->branchDownloadRegions()});
        }
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
                    recordAllPlannedDownload();
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
    recordAllPlannedDownload();

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
    m_stage->forEachUpload([&](int i) {
        auto slot  = m_stage->slot(i);
        auto posAc = chToAc(slot.targetCh, worldTexSizeMask);
        auto& activeChunk = actMgr.activeChunkAtIndex(acToIndex(posAc, worldTexCh));
        // Signal that the chunk is active
        activeChunk = slot.targetCh;
    });

    // Finalize downloads from previous step
    m_stage->forEachDownload([&](int i) {
        auto slot  = m_stage->slot(i);
        auto posAc = chToAc(slot.targetCh, worldTexSizeMask);
        auto& activeChunk = actMgr.activeChunkAtIndex(acToIndex(posAc, worldTexCh));

        // Copy the tiles aside from the stage
        actMgr.addInactiveChunk(
            slot.targetCh,
            Chunk{slot.targetCh, m_stage->tiles(i), m_stage->branchesSerializedSpan(slot)}
        );

        // Signal that there is no active chunk at the spot
        activeChunk = k_chunkNotActive;
    });

    int nTransparentChanges = m_stage->numberOfUploads();
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
    // TODO
    // m_stage->insertUpload(posCh, posAt, tiles.data(), branchesSerialized);
}

void ChunkTransferMgr::planDownload(
    glm::ivec2 posCh, glm::ivec2 posAt, glm::uint branchReadBuf
) {
    BranchRange range{};
    int         allocI = allocIndex(tiToCh(posAt));
    if (allocI >= 0) {
        const auto& alloc = m_regBuf->allocations[allocI];
        range.begin       = alloc.firstBranch;
        range.count       = alloc.branchCount;
    }
    m_stage->insertDownload(posCh, posAt, range, branchReadBuf);
}

void ChunkTransferMgr::endStep(
    const re::CommandBuffer& cmdBuf,
    const re::Texture&       worldTex,
    const re::Buffer&        branchBuf,
    glm::ivec2               worldTexMaskCh
) {
    bool uploadsPlanned = m_stage->numberOfUploads();
    if (uploadsPlanned) {
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

        // Upload tiles
        cmdBuf->copyBufferToImage2(vk::CopyBufferToImageInfo2{
            *m_stage->buffer(),
            worldTex.image(),
            vk::ImageLayout::eGeneral,
            m_stage->tileUploadRegions()});

        // Upload branches
        if (m_stage->numberOfBranchUploads()) {
            cmdBuf->copyBuffer2(vk::CopyBufferInfo2{
                *branchBuf, *m_stage->buffer(), m_stage->branchUploadRegions()});
        }
    }

    bool downloadsPlanned = m_stage->numberOfDownloads();
    if (downloadsPlanned) {
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

        // Download tiles
        cmdBuf->copyImageToBuffer2(vk::CopyImageToBufferInfo2{
            worldTex.image(),
            vk::ImageLayout::eGeneral,
            *m_stage->buffer(),
            m_stage->tileDownloadRegions()});

        // Download branches
        if (m_stage->numberOfBranchDownloads()) {
            cmdBuf->copyBuffer2(vk::CopyBufferInfo2{
                *branchBuf, *m_stage->buffer(), m_stage->branchDownloadRegions()});
        }
    }

    if (uploadsPlanned || downloadsPlanned) {
        // Compose (de)allocation request
        auto allocReq = m_stage->composeBranchAllocRequest(m_worldTexCh);
        cmdBuf->updateBuffer(*m_allocReqBuf, 0, sizeof(BranchAllocReqUB), &allocReq);
        auto barrier = re::bufferMemoryBarrier(
            S::eTransfer,      // Src stage mask
            A::eTransferWrite, // Src access mask
            S::eComputeShader, // Dst stage mask
            A::eUniformRead,   // Dst access mask
            *m_allocReqBuf
        );
        cmdBuf->pipelineBarrier2({{}, {}, barrier, {}});

        // Allocate and deallocate branches
        cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_allocBranchesPl);
        cmdBuf->dispatch(1, 1, 1);
    }
}

void ChunkTransferMgr::downloadBranchAllocRegister(
    const re::CommandBuffer& cmdBuf, const re::Buffer& branchAllocRegBuf
) {
    vk::BufferCopy2 copyRegion{0, 0, sizeof(*m_regBuf.mapped())};
    cmdBuf->copyBuffer2({*branchAllocRegBuf, m_regBuf.buffer(), copyRegion});
}

int ChunkTransferMgr::allocIndex(glm::ivec2 posAc) const {
    return m_regBuf->allocIndexOfTheChunk[acToIndex(posAc, m_worldTexCh)];
}

int ChunkTransferMgr::branchCount(glm::ivec2 posAc) const {
    if (int allocI = allocIndex(posAc); allocI >= 0) {
        return m_regBuf->allocations[allocI].branchCount;
    }
    return 0;
}

} // namespace rw
