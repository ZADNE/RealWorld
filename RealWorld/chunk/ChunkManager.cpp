/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/chunk/ChunkManager.hpp>

#include <iostream>
#include <algorithm>

#include <RealWorld/save/ChunkLoader.hpp>


ChunkManager::ChunkManager(ChunkGenerator& chunkGen, const RE::ShaderProgram& transformShader, GLuint activeChunksInterfaceBlockIndex):
	m_chunkGen(chunkGen) {
	m_activeChunksSSBO.connectToShaderProgram(transformShader, activeChunksInterfaceBlockIndex);
	m_activeChunksSSBO.connectToShaderProgram(m_continuityAnalyzerShader, 0u);
	m_activeChunksSSBO.bind<RE::BufferType::DISPATCH_INDIRECT>();
}

ChunkManager::~ChunkManager() {

}

void ChunkManager::setTarget(int seed, std::string folderPath, RE::Surface* ws) {
	m_folderPath = folderPath;
	m_chunkGen.setSeed(seed);
	m_ws = ws;

	//Reset active chunks
	for (glm::ivec2& ch : m_activeChunks) {
		ch = NO_ACTIVE_CHUNK;
	}

	//Reset SSBO
	auto* ssbo = m_activeChunksSSBO.map<ActiveChunksSSBO>(0, sizeof(ActiveChunksSSBO), WRITE | INVALIDATE_BUFFER);
	for (glm::ivec2& ch : ssbo->activeChunksCh) {//Clear active chunks
		ch = NO_ACTIVE_CHUNK;
	}
	ssbo->dynamicsGroupSize = glm::ivec4{0, 1, 1, 1};
	m_activeChunksSSBO.unmap();

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
			saveChunk(downloadChunk(chToAt(posCh)), posCh);
		}
	}
	return true;
}

size_t ChunkManager::getNumberOfInactiveChunks() {
	return m_inactiveChunks.size();
}

void ChunkManager::step() {
	for (auto it = m_inactiveChunks.begin(); it != m_inactiveChunks.end();) {//For each inactive chunk
		if (it->second.step() >= PHYSICS_STEPS_PER_SECOND * 180) {//If the chunk has not been used for 3 minutes
			saveChunk(it->second.data(), it->first);//Save the chunk to disk
			it = m_inactiveChunks.erase(it);//And remove it from the collection
		} else { it++; }
	}
}

int ChunkManager::forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
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
		m_continuityAnalyzerShader.dispatchCompute({1, 1, 1}, true);
	}
	return activatedChunks;
}

int ChunkManager::activateChunk(const glm::ivec2& posCh) {
	//Check if it is not already active
	auto acIndex = acToIndex(chToAc(posCh));
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
			m_chunkGen.generateChunk(posCh, m_ws->getTexture(), chToAt(posCh));
		}
	}

	//The chunk has been uploaded to the world texture
	//Its position also has to be updated in the active chunks buffer
	auto* ssbo = m_activeChunksSSBO.map<glm::ivec2>(acIndex * sizeof(glm::ivec2), sizeof(glm::ivec2), WRITE | INVALIDATE_RANGE);
	*ssbo = posCh;
	m_activeChunksSSBO.unmap();

	//Signal that the chunk has been activated
	return 1;
}

void ChunkManager::deactivateChunk(const glm::ivec2& posCh) {
	//Get the chunk that is to be deactivated
	auto posAc = chToAc(posCh);
	auto& chunk = m_activeChunks[acToIndex(posAc)];

	//It there is a chunk
	if (chunk != NO_ACTIVE_CHUNK) {
		auto tiles = downloadChunk(chToTi(posAc));//Download the chunk
		m_inactiveChunks.emplace(chunk, Chunk(chunk, std::move(tiles)));//Place it among the inactive chunks
		chunk = NO_ACTIVE_CHUNK;//Mark the position to hold no chunk
	}
}

std::vector<unsigned char> ChunkManager::downloadChunk(const glm::ivec2& posAt) const {
	//Allocate memory for the data
	std::vector<unsigned char> tiles;
	tiles.resize(iCHUNK_SIZE.x * iCHUNK_SIZE.y * 4);

	//Bind the framebuffer
	m_ws->setTarget();
	//Data download (CPU stall -> Pixel Buffer Object todo...)
	glReadnPixels(posAt.x, posAt.y, iCHUNK_SIZE.x, iCHUNK_SIZE.y,
		GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, static_cast<GLsizei>(tiles.size()), tiles.data());
	m_ws->resetTarget();

	return tiles;
}

void ChunkManager::uploadChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
	m_ws->getTexture().setTexelsWithinImage(0, chToAt(posCh), iCHUNK_SIZE, chunk.data());
}

void ChunkManager::saveChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
	ChunkLoader::saveChunk(m_folderPath, posCh, iCHUNK_SIZE, chunk);
}
