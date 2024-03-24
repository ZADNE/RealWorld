/*!
 *  @author    Dubsky Tomas
 */
#include <algorithm>
#include <execution>

#include <RealWorld/save/ChunkLoader.hpp>
#include <RealWorld/world/ChunkActivationMgr.hpp>
#include <RealWorld/world/shaders/AllShaders.hpp>

using enum vk::BufferUsageFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

namespace {
vk::DeviceSize calcActiveChunksBufSize(glm::ivec2 worldTexSizeCh) {
    glm::ivec2 maxContinuous = worldTexSizeCh - 1;
    return sizeof(ActiveChunksSB) +
           sizeof(glm::ivec2) * (maxContinuous.x * maxContinuous.y +
                                 worldTexSizeCh.x * worldTexSizeCh.y);
}
} // namespace

ChunkActivationMgr::ChunkActivationMgr(const re::PipelineLayout& pipelineLayout)
    : m_analyzeContinuityPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::ChunkActivationMgr::analyzeContinuity"},
          {.comp = analyzeContinuity_comp}
      )
    , m_chunkTransferMgr(pipelineLayout) {
}

ChunkActivationMgr::ActivationBuffers ChunkActivationMgr::setTarget(
    const TargetInfo& targetInfo
) {
    m_folderPath        = targetInfo.folderPath;
    m_worldTex          = &targetInfo.worldTex;
    m_branchBuf         = &targetInfo.branchBuf;
    m_branchAllocRegBuf = &targetInfo.branchAllocRegBuf;

    // Recalculate active chunks mask and analyzer dispatch size
    m_worldTexMaskCh              = targetInfo.worldTexCh - 1;
    m_analyzeContinuityGroupCount = targetInfo.worldTexCh / k_minWorldTexSizeCh;

    // Reset ActiveChunks storage buffer
    vk::DeviceSize bufSize = calcActiveChunksBufSize(targetInfo.worldTexCh);
    m_activeChunksBuf      = re::Buffer{re::BufferCreateInfo{
             .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
             .sizeInBytes = bufSize,
             .usage       = eStorageBuffer | eIndirectBuffer | eTransferDst,
             .debugName   = "rw::ChunkActivationMgr::activeChunks"
    }};
    m_activeChunksStageBuf = re::BufferMapped<ActiveChunksSB>{re::BufferCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                      vma::AllocationCreateFlagBits::eHostAccessRandom,
        .sizeInBytes = bufSize,
        .usage       = eTransferSrc,
        .debugName   = "rw::ChunkActivationMgr::activeChunksStage"
    }};
    m_activeChunksStageBuf->activeChunksMask  = m_worldTexMaskCh;
    m_activeChunksStageBuf->worldTexSizeCh    = targetInfo.worldTexCh;
    m_activeChunksStageBuf->dynamicsGroupSize = glm::ivec4{0, 1, 1, 0};
    int maxNumberOfUpdateChunks = m_worldTexMaskCh.x * m_worldTexMaskCh.y;
    int lastChunkIndex          = maxNumberOfUpdateChunks +
                         targetInfo.worldTexCh.x * targetInfo.worldTexCh.y;
    for (int i = maxNumberOfUpdateChunks; i < lastChunkIndex; i++) {
        m_activeChunksStageBuf->offsets[i] = k_chunkNotActive;
    }
    re::CommandBuffer::doOneTimeSubmit([&](const re::CommandBuffer& cb) {
        // Copy whole buffer
        vk::BufferCopy2 bufferCopy{0ull, 0ull, bufSize};
        cb->copyBuffer2(vk::CopyBufferInfo2{
            m_activeChunksStageBuf.buffer(), // Src buffer
            m_activeChunksBuf.buffer(),      // Dst buffer
            bufferCopy                       // Region
        });
    });
    targetInfo.descriptorSet.write(
        vk::DescriptorType::eStorageBuffer, 1u, 0u, m_activeChunksBuf
    );

    m_chunkGen.setTarget(ChunkGenerator::TargetInfo{
        .seed              = targetInfo.seed,
        .worldTex          = targetInfo.worldTex,
        .worldTexSizeCh    = targetInfo.worldTexCh,
        .bodiesBuf         = targetInfo.bodiesBuf,
        .branchBuf         = targetInfo.branchBuf,
        .branchAllocRegBuf = targetInfo.branchAllocRegBuf
    });

    m_chunkTransferMgr.setTarget(targetInfo.worldTexCh);

    return ActivationBuffers{m_activeChunksBuf, m_chunkTransferMgr.allocReqBuf()};
}

bool ChunkActivationMgr::saveChunks() {
    // Save all inactive chunks
    std::for_each(
        std::execution::par_unseq, m_inactiveChunks.begin(),
        m_inactiveChunks.end(),
        [&](const auto& pair) {
            saveChunk(
                pair.first, pair.second.tiles().data(),
                pair.second.branchesSerialized()
            );
        }
    );

    m_inactiveChunks.clear();

    // Save all chunks inside the world texture
    return m_chunkTransferMgr.saveChunks(
        *m_worldTex, *m_branchBuf, m_worldTexMaskCh + 1, *this
    );
}

size_t ChunkActivationMgr::numberOfInactiveChunks() {
    return m_inactiveChunks.size();
}

void ChunkActivationMgr::activateArea(
    const ActionCmdBuf& acb, glm::ivec2 botLeftTi, glm::ivec2 topRightTi
) {
    auto dbg = acb->createDebugRegion("activation manager");
    // Check inactive chunks that have been inactive for too long
    for (auto it = m_inactiveChunks.begin(); it != m_inactiveChunks.end();) {
        // If the inactive chunk has not been used for a minute
        if (it->second.step() >= k_physicsStepsPerSecond * 60) {
            // Save the chunk to disk
            saveChunk(
                it->first, it->second.tiles().data(),
                it->second.branchesSerialized()
            );
            // And remove it from the collection
            it = m_inactiveChunks.erase(it);
        } else {
            it++;
        }
    }

    // Finish transfers from previous step
    // Accumulate changes since last analysis
    m_transparentChunkChanges +=
        m_chunkTransferMgr.beginStep(m_worldTexMaskCh + 1, *this);

    glm::ivec2 botLeftCh  = tiToCh(botLeftTi);
    glm::ivec2 topRightCh = tiToCh(topRightTi);

    // Activate all chunks that at least partially overlap the area
    for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
        for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
            planTransition(glm::ivec2(x, y));
        }
    }

    // Record planned generation
    int chunksGenerated = m_chunkGen.generate(acb);
    m_transparentChunkChanges += chunksGenerated;

    // Record planned uploads/downloads
    m_chunkTransferMgr.endStep(
        acb, *m_worldTex, *m_branchBuf, *m_branchAllocRegBuf, m_worldTexMaskCh,
        chunksGenerated
    );

    // If there have been transparent changes
    // And it is the 'activation' step
    if (m_transparentChunkChanges > 0 &&
        re::StepDoubleBufferingState::writeIndex()) {
        analyzeAfterChanges(acb);
        m_transparentChunkChanges = 0;
    }
}

glm::ivec2& ChunkActivationMgr::activeChunkAtIndex(int acIndex) {
    return m_activeChunksStageBuf
        ->offsets[m_worldTexMaskCh.x * m_worldTexMaskCh.y + acIndex];
}

void ChunkActivationMgr::addInactiveChunk(glm::ivec2 posCh, Chunk&& chunk) {
    m_inactiveChunks.emplace(posCh, std::move(chunk));
}

void ChunkActivationMgr::saveChunk(
    glm::ivec2 posCh, const uint8_t* tiles, std::span<const uint8_t> branchesSerialized
) const {
    ChunkLoader::saveChunk(m_folderPath, posCh, tiles, branchesSerialized);
}

void ChunkActivationMgr::planTransition(glm::ivec2 posCh) {
    auto posAc     = chToAc(posCh, m_worldTexMaskCh);
    auto& activeCh = activeChunkAtIndex(acToIndex(posAc, m_worldTexMaskCh + 1));
    if (activeCh == posCh) {
        // Chunk has already been active
        return; // No transition is needed
    } else if (activeCh == k_chunkNotActive) {
        // No chunk is active at the spot
        planActivation(activeCh, posCh, posAc);
    } else if (!isSpecialChunk(activeCh)) {
        // A different chunk is active at the spot
        planDeactivation(activeCh, posAc);
    }
}

void ChunkActivationMgr::planActivation(
    glm::ivec2& activeCh, glm::ivec2 posCh, glm::ivec2 posAc
) {
    using enum ChunkTransferMgr::UploadPlan;
    // Try to find the chunk among inactive chunks
    if (auto it = m_inactiveChunks.find(posCh); it != m_inactiveChunks.end()) {
        // Query upload of the chunk
        auto res = m_chunkTransferMgr.planUpload(
            posCh, chToTi(posAc), it->second.tiles(),
            it->second.branchesSerialized()
        );
        switch (res) {
        case UploadPlanned:
            // Remove the chunk from inactive chunks
            m_inactiveChunks.erase(it);
            // And signal that it is being uploaded
            activeCh = k_chunkBeingUploaded;
            break;
        case AllocationPlanned:
            // Chunk remains inactive
            activeCh = k_chunkBeingAllocated;
            break;
        }
    } else {
        auto maybeChunk = ChunkLoader::loadChunk(m_folderPath, posCh);
        if (maybeChunk.has_value()) { // If chunk has been loaded
            auto res = m_chunkTransferMgr.planUpload(
                posCh, chToTi(posAc), maybeChunk->tiles(),
                maybeChunk->branchesSerialized()
            );
            switch (res) {
            case UploadPlanned:
                // Signal that it is being uploaded
                activeCh = k_chunkBeingUploaded;
                break;
            case AllocationPlanned:
            case NoUploadSpace:
                // Could not upload the chunk
                // At least store it as an inactive chunk
                m_inactiveChunks.emplace(posCh, std::move(*maybeChunk));
                break;
            }
        } else {
            // Chunk is not on the disk, it has to be generated
            if (m_chunkGen.planGeneration(posCh)) { // If generation could be planned
                activeCh = posCh;
                m_transparentChunkChanges++;
            }
        }
    }
}

void ChunkActivationMgr::planDeactivation(glm::ivec2& activeCh, glm::ivec2 posAc) {
    using enum ChunkTransferMgr::DownloadPlan;
    // Query download of the chunk
    if (m_chunkTransferMgr.planDownload(activeCh, chToTi(posAc)) ==
        DownloadPlanned) {
        activeCh = k_chunkBeingDownloaded; // Deactivate the spot
        m_transparentChunkChanges++;
    }
}

void ChunkActivationMgr::analyzeAfterChanges(const ActionCmdBuf& acb) {
    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Reset the number of update chunks to zero
            m_activeChunksStageBuf->dynamicsGroupSize.x = 0;

            // Copy the update to active chunks buffer
            auto texSizeCh   = m_worldTexMaskCh + 1;
            auto copyRegions = std::to_array<vk::BufferCopy2>(
                {vk::BufferCopy2{
                     offsetof(ActiveChunksSB, dynamicsGroupSize),
                     offsetof(ActiveChunksSB, dynamicsGroupSize),
                     sizeof(m_activeChunksStageBuf->dynamicsGroupSize.x)
                 },
                 vk::BufferCopy2{
                     offsetof(ActiveChunksSB, offsets[0]) +
                         sizeof(ActiveChunksSB::offsets[0]) *
                             m_worldTexMaskCh.x * m_worldTexMaskCh.y,
                     offsetof(ActiveChunksSB, offsets[0]) +
                         sizeof(ActiveChunksSB::offsets[0]) *
                             m_worldTexMaskCh.x * m_worldTexMaskCh.y,
                     sizeof(glm::ivec2) * (texSizeCh.x * texSizeCh.y)
                 }}
            );
            cb->copyBuffer2(vk::CopyBufferInfo2{
                m_activeChunksStageBuf.buffer(), // Src buffer
                m_activeChunksBuf.buffer(),      // Dst buffer
                copyRegions                      // Regions
            });
        },
        BufferAccess{
            .name   = BufferTrackName::ActiveChunks,
            .stage  = S::eTransfer,
            .access = A::eTransferWrite
        }
    );

    acb.action(
        [&](const re::CommandBuffer& cb) {
            // Analyze continuity of the world texture
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeContinuityPl);
            cb->dispatch(
                m_analyzeContinuityGroupCount.x, m_analyzeContinuityGroupCount.y, 1
            );
        },
        BufferAccess{
            .name   = BufferTrackName::ActiveChunks,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        },
        BufferAccess{
            .name   = BufferTrackName::AllocReg,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead | A::eShaderStorageWrite
        }
    );
}

} // namespace rw
