/**
 *  @author    Dubsky Tomas
 */
#include <future>

#include <RealEngine/graphics/synchronization/Fence.hpp>
#include <RealEngine/utility/Math.hpp>

#include <RealWorld/constants/vegetation.hpp>
#include <RealWorld/simulation/tiles/ChunkActivationMgr.hpp>
#include <RealWorld/simulation/tiles/ChunkTransferMgr.hpp>
#include <RealWorld/simulation/tiles/shaders/AllShaders.hpp>
#include <RealWorld/simulation/vegetation/shaders/reallocBranches_comp.hpp>

using enum vk::CommandBufferUsageFlagBits;
using enum vk::ImageAspectFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

using enum ChunkTransferMgr::DownloadPlan;
using enum ChunkTransferMgr::UploadPlan;

ChunkTransferMgr::ChunkTransferMgr(const re::PipelineLayout& pipelineLayout)
    : m_reallocBranchesPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::ChunkTransferMgr::reallocBranches"},
          {.comp = glsl::reallocBranches_comp}
      ) {
}

void ChunkTransferMgr::setTarget(glm::ivec2 worldTexCh) {
    m_worldTexCh = worldTexCh;
    m_stage.forEach([&](auto& st) { st.setTarget(worldTexCh); });
    *m_regBuf = *createBranchAllocRegSB();
}

bool ChunkTransferMgr::saveChunks(
    const re::Texture& worldTex, const re::Buffer& branchBuf,
    glm::ivec2 worldTexCh, ChunkActivationMgr& actMgr
) {
    // Save all active chunks (they have to be downloaded)
    re::CommandBuffer cb{{.debugName = "rw::ChunkTransferMgr::saveChunks"}};
    re::Fence downloadFinishedFence{{}};
    cb->begin({eOneTimeSubmit});

    {                        // Wait for unrasterization to finish
        auto imageBarrier = re::imageMemoryBarrier(
            S::eAllCommands, // Src stage mask
            A::eMemoryRead | A::eMemoryRead,      // Src access mask
            S::eTransfer,                         // Dst stage mask
            A::eTransferRead,                     // Dst access mask
            vk::ImageLayout::eGeneral,            // Old image layout
            vk::ImageLayout::eTransferSrcOptimal, // New image layout
            worldTex.image(), {eColor, 0, 1, 0, k_tileLayerCount}
        );
        cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    }

    auto saveAllChunksInTileStage = [&] {
        std::array<std::future<void>, k_stageSlotCount> futures{};
        m_stage->forEachDownload([&](int i) {
            futures[i] = std::async(
                std::launch::async,
                [this, i, &stage = m_stage, &actMgr]() {
                    auto slt = stage->slot(i);
                    actMgr.saveChunk(
                        slt.targetCh, stage->tiles(i),
                        stage->branchesSerializedSpan(slt)
                    );
                }
            );
        });
        m_stage->reset();
    };

    auto recordAllPlannedDownload = [&] {
        // Download tiles
        cb->copyImageToBuffer2(vk::CopyImageToBufferInfo2{
            worldTex.image(), vk::ImageLayout::eTransferSrcOptimal,
            *m_stage->buffer(), m_stage->tileDownloadRegions()
        });

        // Download branches
        if (m_stage->numberOfBranchDownloads()) {
            cb->copyBuffer2(vk::CopyBufferInfo2{
                *branchBuf, *m_stage->buffer(), m_stage->branchDownloadRegions()
            });
        }
    };

    for (int y = 0; y < worldTexCh.y; ++y) {
        for (int x = 0; x < worldTexCh.x; ++x) {
            auto posAc = glm::ivec2(x, y);
            auto& activeCh = actMgr.activeChunkAtIndex(acToIndex(posAc, worldTexCh));
            if (activeCh != k_chunkNotActive) {
                if (planDownload(activeCh, chToTi(posAc)) != DownloadPlanned) {
                    // The stage is full
                    recordAllPlannedDownload();
                    cb->end();
                    cb.submitToGraphicsCompQueue(*downloadFinishedFence);
                    downloadFinishedFence.wait();
                    downloadFinishedFence.reset();
                    saveAllChunksInTileStage();
                    cb->begin({eOneTimeSubmit});
                    if (planDownload(activeCh, chToTi(posAc)) != DownloadPlanned) {
                        return false; // Shouild not happen...
                    }
                }
            }
        }
    }
    recordAllPlannedDownload();

    {                     // Transition world texture back to original layout
        auto imageBarrier = re::imageMemoryBarrier(
            S::eTransfer, // Src stage mask
            A::eTransferRead,                        // Src access mask
            S::eAllCommands,                         // Dst stage mask
            {},                                      // Dst access mask
            vk::ImageLayout::eTransferSrcOptimal,    // Old image layout
            vk::ImageLayout::eShaderReadOnlyOptimal, // New image layout
            worldTex.image(), {eColor, 0, 1, 0, k_tileLayerCount}
        );
        cb->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
    }
    cb->end();
    cb.submitToGraphicsCompQueue(*downloadFinishedFence);
    downloadFinishedFence.wait();
    downloadFinishedFence.reset();
    saveAllChunksInTileStage();
    return true;
}

int ChunkTransferMgr::beginStep(glm::ivec2 worldTexCh, ChunkActivationMgr& actMgr) {
    auto chToActiveCh = [&](glm::ivec2 posCh) -> glm::ivec2& {
        auto posAc = chToAc(posCh, worldTexCh - 1);
        return actMgr.activeChunkAtIndex(acToIndex(posAc, worldTexCh));
    };

    // Finalize allocations
    m_stage->forEachBranchAllocation([&](glm::ivec2 posCh) {
        // Signal that allocation has finished
        auto& activeCh = chToActiveCh(posCh);
        activeCh       = k_chunkNotActive;
    });

    // Finalize uploads from previous step
    m_stage->forEachUpload([&](int i) {
        // Signal that the chunk is active
        auto slot      = m_stage->slot(i);
        auto& activeCh = chToActiveCh(slot.targetCh);
        activeCh       = slot.targetCh;
    });

    // Finalize downloads from previous step
    m_stage->forEachDownload([&](int i) {
        // Signal that there is no active chunk at the spot
        auto slot      = m_stage->slot(i);
        auto& activeCh = chToActiveCh(slot.targetCh);
        activeCh       = k_chunkNotActive;

        // Copy the tiles aside from the stage
        actMgr.addInactiveChunk(
            slot.targetCh,
            Chunk{slot.targetCh, m_stage->tiles(i), m_stage->branchesSerializedSpan(slot)}
        );
    });

    int nTransparentChanges = m_stage->numberOfUploads();
    m_stage->reset();
    return nTransparentChanges;
}

ChunkTransferMgr::UploadPlan ChunkTransferMgr::planUpload(
    glm::ivec2 posCh, glm::ivec2 posAt, const std::vector<uint8_t>& tiles,
    std::span<const uint8_t> branchesSerialized
) {
    int branchCount = static_cast<int>(
        branchesSerialized.size_bytes() / sizeof(BranchSerialized)
    );
    if (branchCount > 0) {
        if (int allocI = allocIndex(tiToCh(posAt)); allocI >= 0) {
            // Branches are already allocated for the chunk
            const auto& alloc = m_regBuf->allocations[allocI];
            assert(branchCount == alloc.branchCount);
            BranchRange range{
                .begin = static_cast<int>(alloc.firstBranch), .count = branchCount
            };
            if (m_stage->insertUpload(
                    posCh, posAt, range, tiles.data(), branchesSerialized
                )) {
                return UploadPlanned;
            }
        } else {
            // Space for branches must be allocated first
            if (m_stage->insertBranchAllocation(posCh, branchCount)) {
                return AllocationPlanned;
            }
        }
    } else {
        // Allocation of branch space is not needed
        if (m_stage->insertUpload(posCh, posAt, BranchRange{}, tiles.data(), {})) {
            return UploadPlanned;
        }
    }
    return NoUploadSpace;
}

ChunkTransferMgr::DownloadPlan ChunkTransferMgr::planDownload(
    glm::ivec2 posCh, glm::ivec2 posAt
) {
    BranchRange range{};
    if (int allocI = allocIndex(tiToCh(posAt)); allocI >= 0) {
        const auto& alloc = m_regBuf->allocations[allocI];
        range.begin       = static_cast<int>(alloc.firstBranch);
        range.count       = static_cast<int>(alloc.branchCount);
    }
    if (m_stage->insertDownloadAndBranchDeallocation(posCh, posAt, range)) {
        return DownloadPlanned;
    }
    return NoDownloadSpace;
}

void ChunkTransferMgr::endStep(
    const ActionCmdBuf& acb, const re::Texture& worldTex,
    const re::Buffer& branchBuf, const re::Buffer& branchAllocRegBuf,
    glm::ivec2 worldTexMaskCh, bool externalBranchAllocChanges
) {
    if (m_stage->numberOfUploads() > 0) {
        acb.action(
            [&](const re::CommandBuffer& cb) {
                // Upload tiles
                cb->copyBufferToImage2(vk::CopyBufferToImageInfo2{
                    *m_stage->buffer(), worldTex.image(),
                    vk::ImageLayout::eTransferDstOptimal,
                    m_stage->tileUploadRegions()
                });
            },
            ImageAccess{
                .name   = ImageTrackName::World,
                .stage  = S::eTransfer,
                .access = A::eTransferWrite,
                .layout = vk::ImageLayout::eTransferDstOptimal
            }
        );

        if (m_stage->numberOfBranchUploads() > 0) {
            acb.action(
                [&](const re::CommandBuffer& cb) {
                    // Upload branches
                    cb->copyBuffer2(vk::CopyBufferInfo2{
                        *m_stage->buffer(), *branchBuf,
                        m_stage->branchUploadRegions()
                    });
                },
                BufferAccess{
                    .name   = BufferTrackName::Branch,
                    .stage  = S::eTransfer,
                    .access = A::eTransferWrite
                }
            );
        }
    }

    if (m_stage->numberOfDownloads() > 0) {
        acb.action(
            [&](const re::CommandBuffer& cb) {
                // Download tiles
                cb->copyImageToBuffer2(vk::CopyImageToBufferInfo2{
                    worldTex.image(), vk::ImageLayout::eTransferSrcOptimal,
                    *m_stage->buffer(), m_stage->tileDownloadRegions()
                });
            },
            ImageAccess{
                .name   = ImageTrackName::World,
                .stage  = S::eTransfer,
                .access = A::eTransferRead,
                .layout = vk::ImageLayout::eTransferSrcOptimal
            }
        );

        if (m_stage->numberOfBranchDownloads() > 0) {
            acb.action(
                [&](const re::CommandBuffer& cb) {
                    // Download branches
                    cb->copyBuffer2(vk::CopyBufferInfo2{
                        *branchBuf, *m_stage->buffer(),
                        m_stage->branchDownloadRegions()
                    });
                },
                BufferAccess{
                    .name   = BufferTrackName::Branch,
                    .stage  = S::eTransfer,
                    .access = A::eTransferRead
                }
            );
        }
    }

    if (m_stage->allocsOrDeallocsPlanned()) {
        acb.action(
            [&](const re::CommandBuffer& cb) {
                // Copy (de)allocation request
                vk::BufferCopy2 copyRegion{
                    offsetof(decltype(m_stage)::Type::StgBuf, branchAllocReq),
                    0, sizeof(glsl::BranchAllocReqUB)
                };
                cb->copyBuffer2({*m_stage->buffer(), *m_allocReqBuf, copyRegion});
                auto barrier = re::bufferMemoryBarrier(
                    S::eTransfer,      // Src stage mask
                    A::eTransferWrite, // Src access mask
                    S::eComputeShader, // Dst stage mask
                    A::eUniformRead,   // Dst access mask
                    *m_allocReqBuf
                );
                cb->pipelineBarrier2({{}, {}, barrier, {}});

                // Allocate and deallocate branches
                cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_reallocBranchesPl);
                cb->dispatch(1, 1, 1);
            },
            BufferAccess{
                .name   = BufferTrackName::AllocReg,
                .stage  = S::eComputeShader,
                .access = A::eShaderStorageRead | A::eShaderStorageWrite
            }
        );
    }

    if (m_stage->allocsOrDeallocsPlanned() || externalBranchAllocChanges) {
        acb.action(
            [&](const re::CommandBuffer& cb) {
                // Download new state of branch allocation register
                vk::BufferCopy2 region{0, 0, sizeof(glsl::BranchAllocRegSB)};
                cb->copyBuffer2({*branchAllocRegBuf, m_regBuf.buffer(), region});
            },
            BufferAccess{
                .name   = BufferTrackName::AllocReg,
                .stage  = S::eTransfer,
                .access = A::eTransferRead
            }
        );
    }
}

int ChunkTransferMgr::allocIndex(glm::ivec2 posAc) const {
    return m_regBuf->allocIndexOfTheChunk[acToIndex(posAc, m_worldTexCh)];
}

int ChunkTransferMgr::branchCount(glm::ivec2 posAc) const {
    if (int allocI = allocIndex(posAc); allocI >= 0) {
        return static_cast<int>(m_regBuf->allocations[allocI].branchCount);
    }
    return 0;
}

} // namespace rw
