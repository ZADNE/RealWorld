/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/ChunkManager.hpp>

#include <array>

#include <RealWorld/save/ChunkLoader.hpp>

#include <RealEngine/rendering/synchronization/Fence.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;
using enum vk::ImageAspectFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

vk::DeviceSize calcActiveChunksBufSize(const glm::ivec2& activeChunksArea) {
    glm::ivec2 maxContinuous = activeChunksArea - 1;
    return sizeof(ChunkManager::ActiveChunksSB) +
        sizeof(glm::ivec2) * (maxContinuous.x * maxContinuous.y + activeChunksArea.x * activeChunksArea.y);
}

ChunkManager::ChunkManager(ChunkGenerator& chunkGen, const RE::PipelineLayout& pipelineLayout):
    m_chunkGen(chunkGen),
    m_tilesStageBuf(
        k_tileStageSize * k_chunkByteSize,
        eTransferSrc | eTransferDst,
        eHostVisible | eHostCoherent
    ),
    m_analyzeContinuityPl(
        {.pipelineLayout = *pipelineLayout},
        {.comp = analyzeContinuity_comp}
    ) {
}

const RE::Buffer& ChunkManager::setTarget(
    int seed,
    std::string folderPath,
    const RE::Texture& worldTex,
    RE::DescriptorSet& descriptorSet,
    const glm::ivec2& activeChunksArea
) {
    m_folderPath = folderPath;
    m_chunkGen.setSeed(seed);
    m_worldTex = &worldTex;

    //Recalculate active chunks mask and analyzer dispatch size
    m_activeChunksMask = activeChunksArea - 1;
    m_analyzeContinuityGroupCount = activeChunksArea / 8;

    //Reset ActiveChunks storage buffer
    vk::DeviceSize bufSize = calcActiveChunksBufSize(activeChunksArea);
    m_activeChunksBuf.emplace(bufSize, eStorageBuffer | eIndirectBuffer | eTransferDst, eDeviceLocal);
    m_activeChunksStageBuf.emplace(bufSize, eTransferSrc, eHostVisible | eHostCoherent);
    m_activeChunksStageMapped = m_activeChunksStageBuf->map<ActiveChunksSB>(0u, bufSize);
    m_activeChunksStageMapped->activeChunksMask = m_activeChunksMask;
    m_activeChunksStageMapped->activeChunksArea = activeChunksArea;
    m_activeChunksStageMapped->dynamicsGroupSize = glm::ivec4{0, 1, 1, 0};
    int maxNumberOfUpdateChunks = m_activeChunksMask.x * m_activeChunksMask.y;
    int lastChunkIndex = maxNumberOfUpdateChunks + activeChunksArea.x * activeChunksArea.y;
    for (int i = maxNumberOfUpdateChunks; i < lastChunkIndex; i++) {
        m_activeChunksStageMapped->offsets[i] = k_chunkNotActive;
    }
    RE::CommandBuffer::doOneTimeSubmit([&](const vk::CommandBuffer& commandBuffer) {
        vk::BufferCopy2 bufferCopy{                                     //Copy whole buffer
            0ull,
            0ull,
            bufSize
        };
        commandBuffer.copyBuffer2(vk::CopyBufferInfo2{
            m_activeChunksStageBuf->buffer(),                           //Src buffer
            m_activeChunksBuf->buffer(),                                //Dst buffer
            bufferCopy                                                  //Region
        });
    });
    descriptorSet.write(vk::DescriptorType::eStorageBuffer, 1u, 0u, *m_activeChunksBuf, 0ull, bufSize);

    //Clear remnants of previous world
    m_inactiveChunks.clear();
    m_nextFreeTileStage = 0;

    return *m_activeChunksBuf;
}

bool ChunkManager::saveChunks() {
    //Save all inactive chunks
    for (const auto& pair : m_inactiveChunks) {
        saveChunk(pair.second.tiles().data(), pair.first);
    }

    //Save all active chunks (they have to be downloaded)
    assert(m_nextFreeTileStage == 0);
    RE::CommandBuffer commandBuffer{vk::CommandBufferLevel::ePrimary};
    RE::Fence downloadFinishedFence{vk::FenceCreateFlagBits::eSignaled};
    commandBuffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eAllCommands,                                                //Src stage mask
        {},                                                             //Src access mask
        S::eTransfer,                                                   //Dst stage mask
        A::eTransferRead,                                               //Dst access mask
        vk::ImageLayout::eReadOnlyOptimal,                              //Old image layout
        vk::ImageLayout::eGeneral,                                      //New image layout
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,               //Ownership transition
        m_worldTex->image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u}
    };
    commandBuffer->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

    auto saveAllChunksInTileStage = [&]() {
        for (size_t i = 0; i < m_nextFreeTileStage; ++i) {
            const auto& stageState = m_tileStageStates[i];
            saveChunk(&m_tilesStageMapped[k_chunkByteSize * i], stageState.posCh);
        }
        m_nextFreeTileStage = 0;
    };

    auto worldTexSize = m_activeChunksMask + 1;
    for (int y = 0; y < worldTexSize.y; ++y) {
        for (int x = 0; x < worldTexSize.x; ++x) {
            auto posAc = glm::ivec2(x, y);
            auto& activeChunk = activeChunkAtIndex(acToIndex(posAc, worldTexSize));
            if (activeChunk != k_chunkNotActive) {
                if (!planDownload(*commandBuffer, activeChunk, chToTi(posAc))) {//If stage is full
                    commandBuffer->end();
                    commandBuffer.submitToComputeQueue(*downloadFinishedFence); //Wait for the transfer to finish
                    downloadFinishedFence.wait();
                    downloadFinishedFence.reset();
                    saveAllChunksInTileStage();
                    commandBuffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
                }
            }
        }
    }

    imageBarrier = vk::ImageMemoryBarrier2{
        S::eTransfer,                                                   //Src stage mask
        A::eTransferRead,                                               //Src access mask
        S::eAllCommands,                                                //Dst stage mask
        {},                                                             //Dst access mask
        vk::ImageLayout::eGeneral,                                      //Old image layout
        vk::ImageLayout::eReadOnlyOptimal,                              //New image layout
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,               //Ownership transition
        m_worldTex->image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u}
    };
    commandBuffer->pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});

    commandBuffer->end();
    commandBuffer.submitToComputeQueue(*downloadFinishedFence);         //Waits for the transfer to finish
    downloadFinishedFence.wait();
    downloadFinishedFence.reset();
    saveAllChunksInTileStage();
    return true;
}

size_t ChunkManager::numberOfInactiveChunks() {
    return m_inactiveChunks.size();
}

void ChunkManager::beginStep() {
    //Check inactive chunks that have been inactive for too long
    for (auto it = m_inactiveChunks.begin(); it != m_inactiveChunks.end();) {//For each inactive chunk
        if (it->second.step() >= k_physicsStepsPerSecond * 60) {        //If the chunk has not been used for a minute
            saveChunk(it->second.tiles().data(), it->first);            //Save the chunk to disk
            it = m_inactiveChunks.erase(it);                            //And remove it from the collection
        } else { it++; }
    }

    //Finalize tile transfers from previous step
    m_transparentChunkChanges = 0;
    for (size_t i = 0; i < m_nextFreeTileStage; ++i) {
        const auto& stageState = m_tileStageStates[i];
        auto posAc = chToAc(stageState.posCh, m_activeChunksMask);
        auto& activeChunk = activeChunkAtIndex(acToIndex(posAc, m_activeChunksMask + 1));
        switch (stageState.transfer) {
        case TileStageTransferState::Downloading:
        {
            //Copy the tiles aside from the stage
            m_inactiveChunks.emplace(
                stageState.posCh,
                Chunk{stageState.posCh, &m_tilesStageMapped[k_chunkByteSize * i]}
            );
            //Signal that there is no active chunk at the spot
            activeChunk = k_chunkNotActive;
            break;
        }
        case TileStageTransferState::Uploading:
        {
            //Signal the new active chunk
            activeChunk = stageState.posCh;
            m_transparentChunkChanges++;
            break;
        }
        }
    }
    m_nextFreeTileStage = 0;
}

void ChunkManager::planActivationOfChunks(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2& botLeftTi,
    const glm::ivec2& topRightTi
) {
    glm::ivec2 botLeftCh = tiToCh(botLeftTi);
    glm::ivec2 topRightCh = tiToCh(topRightTi);

    //Activate all chunks that at least partially overlap the area
    for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
        for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
            planTransition(commandBuffer, glm::ivec2(x, y));
        }
    }
}

int ChunkManager::endStep(const vk::CommandBuffer& commandBuffer) {
    if (m_transparentChunkChanges > 0) {//If there have been transparent changes
        //Reset the number of update chunks to zero
        m_activeChunksStageMapped->dynamicsGroupSize.x = 0;
        //Copy the update active chunks storage buffer
        auto texSizeCh = m_activeChunksMask + 1;
        auto copyRegions = std::to_array<vk::BufferCopy2>({
            vk::BufferCopy2{
                offsetof(ActiveChunksSB, dynamicsGroupSize),
                offsetof(ActiveChunksSB, dynamicsGroupSize),
                sizeof(m_activeChunksStageMapped->dynamicsGroupSize.x)
            },vk::BufferCopy2{
                offsetof(ActiveChunksSB, offsets[m_activeChunksMask.x * m_activeChunksMask.y]),
                offsetof(ActiveChunksSB, offsets[m_activeChunksMask.x * m_activeChunksMask.y]),
                sizeof(glm::ivec2) * (texSizeCh.x * texSizeCh.y)
            }
        });
        commandBuffer.copyBuffer2(vk::CopyBufferInfo2{
            m_activeChunksStageBuf->buffer(),                           //Src buffer
            m_activeChunksBuf->buffer(),                                //Dst buffer
            copyRegions                                                 //Regions
        });
        //Wait for the copy to finish
        auto bufferBarrier = vk::BufferMemoryBarrier2{
            S::eTransfer,                                               //Src stage mask
            A::eTransferWrite,                                          //Src access mask
            S::eComputeShader,                                          //Dst stage mask
            A::eShaderStorageRead | A::eShaderStorageWrite,             //Dst access mask
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,           //Ownership transition
            m_activeChunksBuf->buffer(),
            offsetof(ActiveChunksSB, dynamicsGroupSize),                //Offset
            VK_WHOLE_SIZE                                               //Size
        };
        commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, bufferBarrier, {}});
        //And analyze the world texture
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeContinuityPl);
        commandBuffer.dispatch(m_analyzeContinuityGroupCount.x, m_analyzeContinuityGroupCount.y, 1);
    }
    return m_transparentChunkChanges;
}

void ChunkManager::planTransition(const vk::CommandBuffer& commandBuffer, const glm::ivec2& posCh) {
    auto posAc = chToAc(posCh, m_activeChunksMask);
    auto& activeChunk = activeChunkAtIndex(acToIndex(posAc, m_activeChunksMask + 1));
    if (activeChunk == posCh) {
        //Chunk has already been active
        return;//No transition is needed
    } else if (activeChunk == k_chunkNotActive) {
        //No chunk is active at the spot
        planActivation(commandBuffer, activeChunk, posCh, chToTi(posAc));
    } else if (activeChunk != k_chunkBeingDownloaded && activeChunk != k_chunkBeingUploaded) {
        //A different chunk is active at the spot
        planDeactivation(commandBuffer, activeChunk, chToTi(posAc));
    }
}

void ChunkManager::planActivation(
    const vk::CommandBuffer& commandBuffer,
    glm::ivec2& activeChunk,
    const glm::ivec2& posCh,
    const glm::ivec2& posAt
) {
    //Try to find the chunk among inactive chunks
    auto it = m_inactiveChunks.find(posCh);
    if (it != m_inactiveChunks.end()) {
        //Query upload of the chunk
        if (planUpload(commandBuffer, it->second.tiles(), posCh, posAt)) {
            m_inactiveChunks.erase(it);         //Remove the chunk from inactive chunks
            activeChunk = k_chunkBeingUploaded; //And signal that it is being uploaded
        }
    } else {
        std::optional<std::vector<uint8_t>> tiles = ChunkLoader::loadChunk(m_folderPath, posCh, iChunkTi);
        if (tiles) {//If chunk has been loaded
            if (planUpload(commandBuffer, *tiles, posCh, posAt)) {
                activeChunk = k_chunkBeingUploaded;  //Signal that it is being uploaded
            } else {
                //Could not upload the chunk -> at least store it as an inactive chunk
                m_inactiveChunks.emplace(posCh, Chunk{posCh, std::move(*tiles)});
            }
        } else {
            //Chunk is not on the disk, it has to be generated
            m_chunkGen.generateChunk(commandBuffer, posCh, *m_worldTex, posAt);
            activeChunk = posCh;
            m_transparentChunkChanges++;
        }
    }
}

void ChunkManager::planDeactivation(
    const vk::CommandBuffer& commandBuffer,
    glm::ivec2& activeChunk,
    const glm::ivec2& posAt
) {
    //Query download of the chunk
    if (planDownload(commandBuffer, activeChunk, posAt)) {
        activeChunk = k_chunkBeingDownloaded;//Deactivate the spot
        m_transparentChunkChanges++;
    }
}

bool ChunkManager::planUpload(
    const vk::CommandBuffer& commandBuffer,
    const std::vector<unsigned char>& tiles,
    const glm::ivec2& posCh,
    const glm::ivec2& posAt
) {
    if (m_nextFreeTileStage < k_tileStageSize) {//If there is a free stage
        m_tileStageStates[m_nextFreeTileStage] = {
            .transfer = TileStageTransferState::Uploading,
            .posCh = posCh
        };
        auto bufOffset = static_cast<vk::DeviceSize>(m_nextFreeTileStage) * k_chunkByteSize;
        std::memcpy(&m_tilesStageMapped[bufOffset], tiles.data(), k_chunkByteSize);
        auto copy = vk::BufferImageCopy2{
            bufOffset,                                                  //Buffer offset
            0u, 0u,                                                     //Tightly packed
            {eColor, 0u, 0u, 1u},                                       //Subresource
            {posAt.x, posAt.y, 0u},                                     //Offset
            {iChunkTi.x, iChunkTi.y, 1u}                                //Extent
        };
        commandBuffer.copyBufferToImage2(
            vk::CopyBufferToImageInfo2{
                m_tilesStageBuf.buffer(),
                m_worldTex->image(),
                vk::ImageLayout::eGeneral,
                copy
            }
        );
        m_nextFreeTileStage++;
        return true;//Upload querried
    }
    return false;//No free stage to do the transfer
}

bool ChunkManager::planDownload(
    const vk::CommandBuffer& commandBuffer,
    const glm::ivec2& posCh,
    const glm::ivec2& posAt
) {
    if (m_nextFreeTileStage < k_tileStageSize) {//If there is a free stage
        m_tileStageStates[m_nextFreeTileStage] = {
            .transfer = TileStageTransferState::Downloading,
            .posCh = posCh
        };
        vk::DeviceSize bufOffset = m_nextFreeTileStage * k_chunkByteSize;
        auto copy = vk::BufferImageCopy2{
            bufOffset,                                                  //Buffer offset
            0u, 0u,                                                     //Tightly packed
            {eColor, 0u, 0u, 1u},                                       //Subresource
            {posAt.x, posAt.y, 0u},                                     //Offset
            {iChunkTi.x, iChunkTi.y, 1u}                                //Extent
        };
        commandBuffer.copyImageToBuffer2(
            vk::CopyImageToBufferInfo2{
                m_worldTex->image(),
                vk::ImageLayout::eGeneral,
                m_tilesStageBuf.buffer(),
                copy
            }
        );
        m_nextFreeTileStage++;
        return true;//Download querried
    }
    return false;//No free stage to do the transfer
}

void ChunkManager::saveChunk(const uint8_t* tiles, glm::ivec2 posCh) const {
    ChunkLoader::saveChunk(m_folderPath, posCh, iChunkTi, tiles);
}

glm::ivec2& ChunkManager::activeChunkAtIndex(int acIndex) const {
    return m_activeChunksStageMapped->offsets[m_activeChunksMask.x * m_activeChunksMask.y + acIndex];
}
