#include <RealWorld/world/chunk/ChunkManager.hpp>

#include <iostream>
#include <algorithm>

#include <RealWorld/world/chunk/ChunkLoader.hpp>
#include <RealWorld/div.hpp>


ChunkManager::ChunkManager(const RE::ShaderProgram& transformShader, GLuint activeChunksInterfaceBlockIndex) {
	m_activeChunksSSBO.connectToShaderProgram(transformShader, activeChunksInterfaceBlockIndex);
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
	ssbo->dynamicsGroupSize = glm::ivec4{1, 1, 1, 1};

	for (int i = 0; i < ACTIVE_CHUNKS_MAX_UPDATES; ++i) {//TEMP!!!
		ssbo->updateOffsetTi[i] = glm::ivec2(i % (ACTIVE_CHUNKS_AREA_X - 1), i / (ACTIVE_CHUNKS_AREA_X - 1)) * CHUNK_SIZE + CHUNK_SIZE / 2;
	}
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
			auto activePosTi = chunkPosToTexturePos(posCh);
			saveChunk(downloadChunk(activePosTi), posCh);
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

void ChunkManager::forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	glm::ivec2 botLeftCh = floor_div(botLeftTi, CHUNK_SIZE).quot;
	glm::ivec2 topRightCh = floor_div(topRightTi, CHUNK_SIZE).quot;

	//Activate all chunks that at least partially overlap the area
	int activatedChunks = 0;
	for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
		for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
			activatedChunks += activateChunk(glm::ivec2(x, y));
		}
	}

	if (activatedChunks > 0) {//If at least one chunk has been activated

	}
}

glm::ivec2 ChunkManager::chunkPosToTexturePos(glm::ivec2 posCh) const {
	return floor_div(posCh, ACTIVE_CHUNKS_AREA).rem * CHUNK_SIZE;
}

glm::ivec2 ChunkManager::chunkPosToActiveChunkPos(glm::ivec2 posCh) const {
	return floor_div(posCh, ACTIVE_CHUNKS_AREA).rem;
}

int ChunkManager::activateChunk(const glm::ivec2& posCh) {
	//Check if it is not already active
	auto activePosCh = chunkPosToActiveChunkPos(posCh);
	auto activeChunkIndex = activePosCh.y * ACTIVE_CHUNKS_AREA.x + activePosCh.x;
	auto& chunk = m_activeChunks[activeChunkIndex];
	if (chunk == posCh) {
		return 0;//The chunk has already been active
	} else {
		deactivateChunk(posCh);//Deactivate the previous chunk
		chunk = posCh;//Se the new chunk to occupy the slot
	}

	//Try to find it among inactive chunks
	auto it = m_inactiveChunks.find(posCh);
	if (it != m_inactiveChunks.end()) {
		uploadChunk(it->second.data(), posCh);
		m_inactiveChunks.erase(it);//Remove the chunk from inactive chunks
	} else {
		try {//Try to load the chunk from its file
			std::vector<unsigned char> tiles = ChunkLoader::loadChunk(m_folderPath, posCh, CHUNK_SIZE);
			//No exception was thrown, chunk has been loaded
			uploadChunk(tiles, posCh);
		}
		catch (...) {
			//Chunk is not on the disk, it has to be generated
			m_chunkGen.generateChunk(posCh, m_ws->getTexture(), chunkPosToTexturePos(posCh));
		}
	}

	//The chunk has been uploaded to the world texture
	//Its position also has to be updated in the active chunks buffer
	auto* ssbo = m_activeChunksSSBO.map<glm::ivec2>(activeChunkIndex * sizeof(glm::ivec2), sizeof(glm::ivec2), WRITE | INVALIDATE_RANGE);
	*ssbo = posCh;
	m_activeChunksSSBO.unmap();

	//The chunk has been activated
	return 1;
}

void ChunkManager::deactivateChunk(const glm::ivec2& posCh) {
	//Get the chunk that is to be deactivated
	auto activePosCh = chunkPosToActiveChunkPos(posCh);
	auto& chunk = m_activeChunks[activePosCh.y * ACTIVE_CHUNKS_AREA.x + activePosCh.x];

	//It there is a chunk
	if (chunk != NO_ACTIVE_CHUNK) {
		auto tiles = downloadChunk(activePosCh * CHUNK_SIZE);//Dwonload the chunk
		m_inactiveChunks.emplace(chunk, Chunk(chunk, std::move(tiles)));//Place it among the inactive chunks
		chunk = NO_ACTIVE_CHUNK;//Mark the position to hold no chunk
	}
}

std::vector<unsigned char> ChunkManager::downloadChunk(const glm::ivec2& activePosTi) const {
	//Allocate memory for the data
	std::vector<unsigned char> tiles;
	tiles.resize(CHUNK_SIZE.x * CHUNK_SIZE.y * 4);

	//Bind the framebuffer
	m_ws->setTarget();
	//Data download (CPU stall -> Pixel Buffer Object todo...)
	glReadnPixels(activePosTi.x, activePosTi.y, CHUNK_SIZE.x, CHUNK_SIZE.y,
		GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, static_cast<GLsizei>(tiles.size()), tiles.data());
	m_ws->resetTarget();

	return tiles;
}

void ChunkManager::uploadChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
	glm::ivec2 texturePos = chunkPosToTexturePos(posCh);
	m_ws->getTexture().setTexelsWithinImage(0, texturePos, CHUNK_SIZE, chunk.data());
}

void ChunkManager::saveChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const {
	ChunkLoader::saveChunk(m_folderPath, posCh, CHUNK_SIZE, chunk);
}
