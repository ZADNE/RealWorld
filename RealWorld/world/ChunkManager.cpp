/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/ChunkManager.hpp>

#include <algorithm>

#include <RealWorld/save/ChunkLoader.hpp>

int calcActiveChunksBufSize(const glm::ivec2& activeChunksArea) {
    glm::ivec2 maxContinuous = activeChunksArea - 1;
    return sizeof(glm::ivec4) +
        sizeof(glm::ivec2) * (activeChunksArea.x * activeChunksArea.y + maxContinuous.x * maxContinuous.y);
}

template<RE::Renderer R>
ChunkManager<R>::ChunkManager(ChunkGenerator<R>& chunkGen) :
    m_chunkGen(chunkGen) {
    m_contAnalyzerShd.backInterfaceBlock(0u, STRG_BUF_ACTIVECHUNKS);
}

template<RE::Renderer R>
void ChunkManager<R>::setTarget(int seed, std::string folderPath, RE::Texture<R>* worldTex) {
    m_folderPath = folderPath;
    m_chunkGen.setSeed(seed);
    m_worldTex = worldTex;

    //Recalculate active chunks mask and analyzer dispatch size
    glm::ivec2 activeChunksArea = m_worldTex->getTrueDims() / uCHUNK_SIZE;
    m_activeChunksMask = activeChunksArea - 1;
    m_contAnalyzerGroupCount = {activeChunksArea / 8, 1};
    m_contAnalyzerShd.setUniform("activeChunksArea", activeChunksArea);
    m_contAnalyzerShd.setUniform("activeChunksMask", m_activeChunksMask);

    //Reset active chunks
    m_activeChunks.clear();
    m_activeChunks.resize(activeChunksArea.x * activeChunksArea.y, NO_ACTIVE_CHUNK);

    //Reset SSBO
    m_activeChunksBuf = RE::BufferTyped<R>{STRG_BUF_ACTIVECHUNKS, calcActiveChunksBufSize(activeChunksArea), MAP_WRITE};
    auto* ssbo = m_activeChunksBuf.template map<ChunkManager<R>::ActiveChunksSSBO>(0, calcActiveChunksBufSize(activeChunksArea), WRITE | INVALIDATE_BUFFER);
    ssbo->dynamicsGroupSize = glm::ivec4{0, 1, 1, 0};
    int maxNumberOfUpdateChunks = m_activeChunksMask.x * m_activeChunksMask.y;
    for (int i = maxNumberOfUpdateChunks; i < (maxNumberOfUpdateChunks + activeChunksArea.x * activeChunksArea.y); i++) {
        ssbo->offsets[i] = NO_ACTIVE_CHUNK;
    }
    m_activeChunksBuf.unmap();
    m_activeChunksBuf.bind(RE::BufferType::DISPATCH_INDIRECT);

    //Clear inactive chunks as they do not belong to this world
    m_inactiveChunks.clear();
}

template<RE::Renderer R>
bool ChunkManager<R>::saveChunks() const {
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

template<RE::Renderer R>
size_t ChunkManager<R>::getNumberOfInactiveChunks() {
    return m_inactiveChunks.size();
}

template<RE::Renderer R>
void ChunkManager<R>::step() {
    for (auto it = m_inactiveChunks.begin(); it != m_inactiveChunks.end();) {//For each inactive chunk
        if (it->second.step() >= PHYSICS_STEPS_PER_SECOND * 60) {//If the chunk has not been used for a minute
            saveChunk(it->second.data(), it->first);//Save the chunk to disk
            it = m_inactiveChunks.erase(it);//And remove it from the collection
        } else { it++; }
    }
}

template<RE::Renderer R>
int ChunkManager<R>::forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
    glm::ivec2 botLeftCh = tiToCh(botLeftTi);
    glm::ivec2 topRightCh = tiToCh(topRightTi);

    //Activate all chunks that at least partially overlap the area
    int activatedChunks = 0;
    for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
        for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
            activatedChunks += activateChunk(glm::ivec2(x, y));
        }
    }

    if (activatedChunks > 0) {//If at least one chunk has been activated
        //Reset the number of update chunks to zero
        auto* numberOfUpdateChunks = m_activeChunksBuf.template map<int>(0, sizeof(int), WRITE | INVALIDATE_RANGE);
        *numberOfUpdateChunks = 0;
        m_activeChunksBuf.unmap();
        //And analyze the world texture
        m_contAnalyzerShd.dispatchCompute(m_contAnalyzerGroupCount, true);
    }
    return activatedChunks;
}

template<RE::Renderer R>
int ChunkManager<R>::activateChunk(const glm::ivec2& posCh) {
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
        }
        catch (...) {
            //Chunk is not on the disk, it has to be generated
            m_chunkGen.generateChunk(posCh, *m_worldTex, chToAt(posCh, m_activeChunksMask));
        }
    }

    //The chunk has been uploaded to the world texture
    //Its position also has to be updated in the active chunks buffer
    int mapOffset = offsetof(ActiveChunksSSBO, offsets) + (m_activeChunksMask.x * m_activeChunksMask.y + acIndex) * sizeof(glm::ivec2);
    auto* ssbo = m_activeChunksBuf.template map<glm::ivec2>(mapOffset, sizeof(glm::ivec2), WRITE | INVALIDATE_RANGE);
    *ssbo = posCh;
    m_activeChunksBuf.unmap();

    //Signal that the chunk has been activated
    return 1;
}

template<RE::Renderer R>
void ChunkManager<R>::deactivateChunk(const glm::ivec2& posCh) {
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

template<RE::Renderer R>
std::vector<unsigned char> ChunkManager<R>::downloadChunk(const glm::ivec2& posAt) const {
    //Copy the chunk to (client-local) pack buffer
    m_downloadBuf.bind(RE::BufferType::PIXEL_PACK);
    m_worldTex->getTexels(0, posAt, iCHUNK_SIZE, m_downloadBuf.size(), nullptr);

    //Map it right away (causes synchronization!)
    auto* pixels = m_downloadBuf.template map<unsigned char>(0u, m_downloadBuf.size(), READ);

    //Copy it the tiles to local vector
    std::vector<unsigned char> tiles;
    tiles.assign(pixels, &pixels[iCHUNK_SIZE.x * iCHUNK_SIZE.y * 4]);
    m_downloadBuf.unmap();

    return tiles;
}

template<RE::Renderer R>
void ChunkManager<R>::uploadChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
    m_worldTex->setTexels(0, chToAt(posCh, m_activeChunksMask), iCHUNK_SIZE, chunk.data());
}

template<RE::Renderer R>
void ChunkManager<R>::saveChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
    ChunkLoader::saveChunk(m_folderPath, posCh, iCHUNK_SIZE, chunk);
}

template class ChunkManager<RE::RendererVK13>;
template class ChunkManager<RE::RendererGL46>;
