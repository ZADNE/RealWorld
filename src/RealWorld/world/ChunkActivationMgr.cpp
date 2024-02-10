/*!
 *  @author    Dubsky Tomas
 */
#include <algorithm>
#include <execution>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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

const re::Buffer& ChunkActivationMgr::setTarget(const TargetInfo& targetInfo) {
    m_folderPath = targetInfo.folderPath;
    m_worldTex   = &targetInfo.worldTex;
    m_branchBuf  = &targetInfo.branchBuf;

    // Recalculate active chunks mask and analyzer dispatch size
    m_worldTexMaskCh              = targetInfo.worldTexCh - 1;
    m_analyzeContinuityGroupCount = targetInfo.worldTexCh / 16;

    // Reset ActiveChunks storage buffer
    vk::DeviceSize bufSize = calcActiveChunksBufSize(targetInfo.worldTexCh);
    m_activeChunksBuf      = re::Buffer{re::BufferCreateInfo{
             .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
             .sizeInBytes = bufSize,
             .usage       = eStorageBuffer | eIndirectBuffer | eTransferDst,
             .debugName   = "rw::ChunkActivationMgr::activeChunks"}};
    m_activeChunksStageBuf = re::BufferMapped<ActiveChunksSB>{re::BufferCreateInfo{
        .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                      vma::AllocationCreateFlagBits::eHostAccessRandom,
        .sizeInBytes = bufSize,
        .usage       = eTransferSrc,
        .debugName   = "rw::ChunkActivationMgr::activeChunksStage"}};
    m_activeChunksStageBuf->activeChunksMask  = m_worldTexMaskCh;
    m_activeChunksStageBuf->worldTexSizeCh    = targetInfo.worldTexCh;
    m_activeChunksStageBuf->dynamicsGroupSize = glm::ivec4{0, 1, 1, 0};
    int maxNumberOfUpdateChunks = m_worldTexMaskCh.x * m_worldTexMaskCh.y;
    int lastChunkIndex          = maxNumberOfUpdateChunks +
                         targetInfo.worldTexCh.x * targetInfo.worldTexCh.y;
    for (int i = maxNumberOfUpdateChunks; i < lastChunkIndex; i++) {
        m_activeChunksStageBuf->offsets[i] = k_chunkNotActive;
    }
    re::CommandBuffer::doOneTimeSubmit([&](const re::CommandBuffer& cmdBuf) {
        // Copy whole buffer
        vk::BufferCopy2 bufferCopy{0ull, 0ull, bufSize};
        cmdBuf->copyBuffer2(vk::CopyBufferInfo2{
            m_activeChunksStageBuf.buffer(), // Src buffer
            m_activeChunksBuf.buffer(),      // Dst buffer
            bufferCopy                       // Region
        });
    });
    targetInfo.descriptorSet.write(
        vk::DescriptorType::eStorageBuffer, 1u, 0u, m_activeChunksBuf, 0ull, bufSize
    );

    m_chunkGen.setTarget(ChunkGenerator::TargetInfo{
        .seed           = targetInfo.seed,
        .worldTex       = targetInfo.worldTex,
        .worldTexSizeCh = targetInfo.worldTexCh,
        .bodiesBuf      = targetInfo.bodiesBuf,
        .branchBuf      = targetInfo.branchBuf});

    m_chunkTransferMgr.reset();

    return m_activeChunksBuf;
}

bool ChunkActivationMgr::saveChunks() {
    // Save all inactive chunks
    std::for_each(
        std::execution::par_unseq,
        m_inactiveChunks.begin(),
        m_inactiveChunks.end(),
        [&](const auto& pair) {
            saveChunk(pair.second.tiles().data(), pair.first);
        }
    );

    m_inactiveChunks.clear();

    // Save all chunks inside the world texture
    return m_chunkTransferMgr.saveChunks(*m_worldTex, m_worldTexMaskCh + 1, *this);
}

size_t ChunkActivationMgr::numberOfInactiveChunks() {
    return m_inactiveChunks.size();
}

void ChunkActivationMgr::activateArea(
    const re::CommandBuffer& cmdBuf,
    glm::ivec2               botLeftTi,
    glm::ivec2               topRightTi,
    glm::uint                branchWriteBuf
) {
    auto dbg = cmdBuf.createDebugRegion("activation manager");
    // Check inactive chunks that have been inactive for too long
    for (auto it = m_inactiveChunks.begin(); it != m_inactiveChunks.end();) {
        // If the inactive chunk has not been used for a minute
        if (it->second.step() >= k_physicsStepsPerSecond * 60) {
            // Save the chunk to disk
            saveChunk(it->second.tiles().data(), it->first);
            // And remove it from the collection
            it = m_inactiveChunks.erase(it);
        } else {
            it++;
        }
    }

    // Finish transfers from previous step
    m_transparentChunkChanges =
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
    m_chunkGen.generate(cmdBuf, branchWriteBuf);

    // Record planned uploads/downloads
    m_chunkTransferMgr.endStep(cmdBuf, *m_worldTex);

    // If there have been transparent changes
    if (m_transparentChunkChanges > 0) {
        analyzeAfterChanges(cmdBuf);
    }
}

glm::ivec2& ChunkActivationMgr::activeChunkAtIndex(int acIndex) {
    return m_activeChunksStageBuf
        ->offsets[m_worldTexMaskCh.x * m_worldTexMaskCh.y + acIndex];
}

void ChunkActivationMgr::addInactiveChunk(glm::ivec2 posCh, Chunk&& chunk) {
    m_inactiveChunks.emplace(posCh, std::move(chunk));
}

void ChunkActivationMgr::saveChunk(const uint8_t* tiles, glm::ivec2 posCh) const {
    ChunkLoader::saveChunk(m_folderPath, posCh, iChunkTi, tiles);
}

void ChunkActivationMgr::planTransition(glm::ivec2 posCh) {
    auto posAc = chToAc(posCh, m_worldTexMaskCh);
    auto& activeChunk = activeChunkAtIndex(acToIndex(posAc, m_worldTexMaskCh + 1));
    if (activeChunk == posCh) {
        // Chunk has already been active
        return; // No transition is needed
    } else if (activeChunk == k_chunkNotActive) {
        // No chunk is active at the spot
        planActivation(activeChunk, posCh, posAc);
    } else if (activeChunk != k_chunkBeingDownloaded && activeChunk != k_chunkBeingUploaded) {
        // A different chunk is active at the spot
        planDeactivation(activeChunk, posAc);
    }
}

void ChunkActivationMgr::planActivation(
    glm::ivec2& activeChunk, glm::ivec2 posCh, glm::ivec2 posAc
) {
    // Try to find the chunk among inactive chunks
    auto it = m_inactiveChunks.find(posCh);
    if (it != m_inactiveChunks.end()) {
        // Query upload of the chunk
        if (m_chunkTransferMgr.hasFreeTransferSpace(posAc)) {
            m_chunkTransferMgr.planUpload(it->second.tiles(), posCh, chToTi(posAc));
            // Remove the chunk from inactive chunks
            m_inactiveChunks.erase(it);
            // And signal that it is being uploaded
            activeChunk = k_chunkBeingUploaded;
        }
    } else {
        auto tiles = ChunkLoader::loadChunk(m_folderPath, posCh, iChunkTi);
        if (tiles.size() > 0) { // If chunk has been loaded
            if (m_chunkTransferMgr.hasFreeTransferSpace(posAc)) {
                m_chunkTransferMgr.planUpload(tiles, posCh, chToTi(posAc));
                // Signal that it is being uploaded
                activeChunk = k_chunkBeingUploaded;
            } else {
                // Could not upload the chunk
                // At least store it as an inactive chunk
                m_inactiveChunks.emplace(posCh, Chunk{posCh, std::move(tiles), {}});
            }
        } else {
            // Chunk is not on the disk, it has to be generated
            if (m_chunkGen.planGeneration(posCh)) { // If generation could be planned
                activeChunk = posCh;
                m_transparentChunkChanges++;
            }
        }
    }
}

void ChunkActivationMgr::planDeactivation(glm::ivec2& activeChunk, glm::ivec2 posAc) {
    // Query download of the chunk
    if (m_chunkTransferMgr.hasFreeTransferSpace(posAc)) {
        m_chunkTransferMgr.planDownload(activeChunk, chToTi(posAc));
        activeChunk = k_chunkBeingDownloaded; // Deactivate the spot
        m_transparentChunkChanges++;
    }
}

void ChunkActivationMgr::analyzeAfterChanges(const re::CommandBuffer& cmdBuf) {
    // Reset the number of update chunks to zero
    m_activeChunksStageBuf->dynamicsGroupSize.x = 0;

    // Copy the update to active chunks buffer
    auto texSizeCh   = m_worldTexMaskCh + 1;
    auto copyRegions = std::to_array<vk::BufferCopy2>(
        {vk::BufferCopy2{
             offsetof(ActiveChunksSB, dynamicsGroupSize),
             offsetof(ActiveChunksSB, dynamicsGroupSize),
             sizeof(m_activeChunksStageBuf->dynamicsGroupSize.x)},
         vk::BufferCopy2{
             offsetof(ActiveChunksSB, offsets[0]) +
                 sizeof(ActiveChunksSB::offsets[0]) * m_worldTexMaskCh.x *
                     m_worldTexMaskCh.y,
             offsetof(ActiveChunksSB, offsets[0]) +
                 sizeof(ActiveChunksSB::offsets[0]) * m_worldTexMaskCh.x *
                     m_worldTexMaskCh.y,
             sizeof(glm::ivec2) * (texSizeCh.x * texSizeCh.y)}}
    );
    cmdBuf->copyBuffer2(vk::CopyBufferInfo2{
        m_activeChunksStageBuf.buffer(), // Src buffer
        m_activeChunksBuf.buffer(),      // Dst buffer
        copyRegions                      // Regions
    });

    { // Wait for the copy to finish
        auto bufferBarrier = re::bufferMemoryBarrier(
            S::eTransfer,                                   // Src stage mask
            A::eTransferWrite,                              // Src access mask
            S::eComputeShader,                              // Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite, // Dst access mask
            m_activeChunksBuf.buffer(),
            offsetof(ActiveChunksSB, dynamicsGroupSize), // Offset
            vk::WholeSize                                // Size
        );
        cmdBuf->pipelineBarrier2({{}, {}, bufferBarrier, {}});
    }

    // Analyze the world texture
    cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeContinuityPl);
    cmdBuf->dispatch(
        m_analyzeContinuityGroupCount.x, m_analyzeContinuityGroupCount.y, 1
    );

    { // Barrier analysis output from tile transformations and alloc register download
        auto bufferBarriers = std::to_array(
            {re::bufferMemoryBarrier(
                 S::eComputeShader,       // Src stage mask
                 A::eShaderStorageWrite,  // Src access mask
                 S::eDrawIndirect,        // Dst stage mask
                 A::eIndirectCommandRead, // Dst access mask
                 m_activeChunksBuf.buffer()
             ),
             re::bufferMemoryBarrier(
                 S::eComputeShader, // Src stage mask
                 A::eShaderStorageRead | A::eShaderStorageWrite, // Src access mask
                 S::eTransfer,     // Dst stage mask
                 A::eTransferRead, // Dst access mask
                 m_branchBuf->buffer()
             )}
        );
        cmdBuf->pipelineBarrier2({{}, {}, bufferBarriers, {}});
    }

    m_chunkTransferMgr.downloadBranchAllocRegister(cmdBuf, *m_branchBuf);
}

} // namespace rw
