/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/ChunkManager.hpp>

#include <algorithm>

#include <RealWorld/save/ChunkLoader.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;

vk::DeviceSize calcActiveChunksBufSize(const glm::ivec2& activeChunksArea) {
    glm::ivec2 maxContinuous = activeChunksArea - 1;
    return sizeof(ActiveChunksSSBO) +
        sizeof(glm::ivec2) * (activeChunksArea.x * activeChunksArea.y + maxContinuous.x * maxContinuous.y);
}

ChunkManager::ChunkManager(ChunkGenerator& chunkGen):
    m_chunkGen(chunkGen),
    m_tilesStageBuf(uCHUNK_SIZE.x* uCHUNK_SIZE.y * 4u, eTransferDst, eHostVisible | eHostCoherent) {
}

void ChunkManager::setTarget(int seed, std::string folderPath, RE::Texture& worldTex, const glm::ivec2& activeChunksArea) {
    m_folderPath = folderPath;
    m_chunkGen.setSeed(seed);
    m_worldTex = &worldTex;

    //Recalculate active chunks mask and analyzer dispatch size
    m_activeChunksMask = activeChunksArea - 1;
    m_analyzeContinuityGroupCount = activeChunksArea / 8;

    //Reset active chunks
    m_activeChunks.clear();
    m_activeChunks.resize(activeChunksArea.x * activeChunksArea.y, NO_ACTIVE_CHUNK);

    //Reset SSBO
    vk::DeviceSize bufSize = calcActiveChunksBufSize(activeChunksArea);
    m_activeChunksBuf.emplace(bufSize, eStorageBuffer | eIndirectBuffer | eTransferDst, eDeviceLocal);
    m_activeChunksStageBuf.emplace(bufSize, eTransferSrc, eHostVisible | eHostCoherent);
    m_activeChunksStageMapped = m_activeChunksStageBuf->map<ActiveChunksSSBO>(0u, bufSize);
    m_activeChunksStageMapped->activeChunksMask = m_activeChunksMask;
    m_activeChunksStageMapped->activeChunksArea = activeChunksArea;
    m_activeChunksStageMapped->dynamicsGroupSize = glm::ivec4{0, 1, 1, 0};
    int maxNumberOfUpdateChunks = m_activeChunksMask.x * m_activeChunksMask.y;
    for (int i = maxNumberOfUpdateChunks; i < (maxNumberOfUpdateChunks + activeChunksArea.x * activeChunksArea.y); i++) {
        m_activeChunksStageMapped->offsets[i] = NO_ACTIVE_CHUNK;
    }
    m_activeChunksStageBuf->copyToBuffer(*m_activeChunksBuf, vk::BufferCopy{0u, 0u, bufSize});
    m_descriptorSet.write(vk::DescriptorType::eStorageBuffer, 0u, 0u, *m_activeChunksBuf, 0ull, bufSize);

    //Clear inactive chunks as they do not belong to this world
    m_inactiveChunks.clear();
}

bool ChunkManager::saveChunks() const {
    //Save all inactive chunks
    for (auto& pair : m_inactiveChunks) {
        saveChunk(pair.second.data(), pair.first);
    }
    //Save all active chunks (they have to be downloaded)
    for (auto& posCh : m_activeChunks) {
        if (posCh != NO_ACTIVE_CHUNK) {
            saveChunk(downloadChunk(chToAt(posCh, m_activeChunksMask)), posCh);
        }
    }
    return true;
}

size_t ChunkManager::getNumberOfInactiveChunks() {
    return m_inactiveChunks.size();
}

void ChunkManager::step() {
    for (auto it = m_inactiveChunks.begin(); it != m_inactiveChunks.end();) {//For each inactive chunk
        if (it->second.step() >= PHYSICS_STEPS_PER_SECOND * 60) {//If the chunk has not been used for a minute
            saveChunk(it->second.data(), it->first);//Save the chunk to disk
            it = m_inactiveChunks.erase(it);//And remove it from the collection
        } else { it++; }
    }
}

int ChunkManager::forceActivationOfChunks(const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
    glm::ivec2 botLeftCh = tiToCh(botLeftTi);
    glm::ivec2 topRightCh = tiToCh(topRightTi);

    //Activate all chunks that at least partially overlap the area
    int activatedChunks = 0;
    for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
        for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
            activatedChunks += activateChunk(commandBuffer, glm::ivec2(x, y));
        }
    }

    if (activatedChunks > 0) {//If at least one chunk has been activated
        //Reset the number of update chunks to zero
        m_activeChunksStageMapped->dynamicsGroupSize.x = 0;
        m_activeChunksStageBuf->copyToBuffer(
            *m_activeChunksBuf,
            vk::BufferCopy{
                offsetof(ActiveChunksSSBO, dynamicsGroupSize),
                offsetof(ActiveChunksSSBO, dynamicsGroupSize),
                sizeof(int)
            }
        );
        //And analyze the world texture
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descriptorSet, {});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_analyzeContinuityPl);
        commandBuffer.dispatch(m_analyzeContinuityGroupCount.x, m_analyzeContinuityGroupCount.y, 1);
    }
    return activatedChunks;
}

int ChunkManager::activateChunk(const vk::CommandBuffer& commandBuffer, const glm::ivec2& posCh) {
    //Check if it is not already active
    auto acIndex = acToIndex(chToAc(posCh, m_activeChunksMask), m_activeChunksMask + 1);
    auto& chunk = m_activeChunks[acIndex];
    if (chunk == posCh) {
        return 0;//Signals that the chunk has already been active
    } else {
        deactivateChunk(posCh);//Deactivate the previous chunk
        chunk = posCh;//Set the new chunk to occupy the slot
    }

    //Try to find it among inactive chunks
    auto it = m_inactiveChunks.find(posCh);
    if (it != m_inactiveChunks.end()) {
        uploadChunk(it->second.data(), posCh);
        m_inactiveChunks.erase(it);//Remove the chunk from inactive chunks
    } else {
        try {//Try to load the chunk from its file
            std::vector<unsigned char> tiles = ChunkLoader::loadChunk(m_folderPath, posCh, iCHUNK_SIZE);
            //No exception was thrown, chunk has been loaded
            uploadChunk(tiles, posCh);
        } catch (...) {
            //Chunk is not on the disk, it has to be generated
            m_chunkGen.generateChunk(commandBuffer, posCh, *m_worldTex, chToAt(posCh, m_activeChunksMask));
        }
    }

    //The chunk has been uploaded to the world texture
    //Its position also has to be updated in the active chunks buffer
    glm::ivec2* offset = &m_activeChunksStageMapped->offsets[m_activeChunksMask.x * m_activeChunksMask.y + acIndex];
    *offset = posCh;
    vk::DeviceSize byteOffset = (char*)offset - (char*)m_activeChunksStageMapped;
    m_activeChunksStageBuf->copyToBuffer(
        *m_activeChunksBuf,
        vk::BufferCopy{
            byteOffset,
            byteOffset,
            sizeof(glm::ivec2)
        }
    );

    //Signal that the chunk has been activated
    return 1;
}

void ChunkManager::deactivateChunk(const glm::ivec2& posCh) {
    //Get the chunk that is to be deactivated
    auto posAc = chToAc(posCh, m_activeChunksMask);
    auto& chunk = m_activeChunks[acToIndex(posAc, m_activeChunksMask + 1)];

    //It there is a chunk
    if (chunk != NO_ACTIVE_CHUNK) {
        auto tiles = downloadChunk(chToTi(posAc));//Download the chunk
        m_inactiveChunks.emplace(chunk, Chunk(chunk, std::move(tiles)));//Place it among the inactive chunks
        chunk = NO_ACTIVE_CHUNK;//Mark the position to hold no chunk
    }
}

std::vector<unsigned char> ChunkManager::downloadChunk(const glm::ivec2& posAt) const {
    //Copy the chunk to (client-local) pack buffer
    //m_tilesStageBuf.bind(RE::BufferType::PIXEL_PACK);
    //m_worldTex->getTexels(0, posAt, iCHUNK_SIZE, m_tilesStageBuf.size(), nullptr);
    //TODO copy to client local and synchronize

    //Copy it the tiles to local vector
    std::vector<unsigned char> tiles;
    tiles.assign(m_tilesStageMapped, &m_tilesStageMapped[iCHUNK_SIZE.x * iCHUNK_SIZE.y * 4]);

    return tiles;
}

void ChunkManager::uploadChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
    //m_worldTex->setTexels(0, chToAt(posCh, m_activeChunksMask), iCHUNK_SIZE, chunk.data());
    std::memcpy(m_tilesStageMapped, chunk.data(), iCHUNK_SIZE.x * iCHUNK_SIZE.y * 4);

    //TODO copy to texture and synchronize
}

void ChunkManager::saveChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
    ChunkLoader::saveChunk(m_folderPath, posCh, iCHUNK_SIZE, chunk);
}
