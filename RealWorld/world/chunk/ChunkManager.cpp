#include <RealWorld/world/chunk/ChunkManager.hpp>

#include <iostream>
#include <algorithm>

#include <RealWorld/world/chunk/ChunkLoader.hpp>
#include <RealWorld/div.hpp>


#include <chrono>


ChunkManager::ChunkManager() :
	m_chunkRemovalThreshold(PHYSICS_STEPS_PER_SECOND * 60) {

}

ChunkManager::~ChunkManager() {

}

void ChunkManager::setTarget(int seed, glm::uvec2 activeChunksRect, std::string folderPath, RE::Surface* ws) {
	m_folderPath = folderPath;
	m_chunkGen.setSeed(seed);
	m_ws = ws;
	m_activeChunks.clear();
	m_activeChunks.resize(static_cast<size_t>(activeChunksRect.x) * activeChunksRect.y, NO_ACTIVE_CHUNK);
	m_inactiveChunks.clear();
	m_activeChunksTex = RE::Texture{RE::Raster{activeChunksRect, RG}, {RG32_IS_NEAR_NEAR_REP}};
	m_activeChunksTex.clear(glm::ivec4(NO_ACTIVE_CHUNK, 0, 0));
}

bool ChunkManager::saveChunks() const {
	for (auto& pair : m_inactiveChunks) {
		saveChunk(pair.second.data(), pair.first);
	}
	for (auto& posCh: m_activeChunks) {
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
	for (auto it = m_inactiveChunks.begin(); it != m_inactiveChunks.end();) {
		if (it->second.step() >= m_chunkRemovalThreshold) {
			deactivateChunk(it->first);
			saveChunk(it->second.data(), it->first);
			it = m_inactiveChunks.erase(it);
		} else { it++; }
	}
}

void ChunkManager::forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	glm::ivec2 botLeftCh = floor_div(botLeftTi, CHUNK_SIZE).quot;
	glm::ivec2 topRightCh = floor_div(topRightTi, CHUNK_SIZE).quot;

	static int N = -7;
	static std::chrono::nanoseconds sum = std::chrono::nanoseconds::zero();
	auto start = std::chrono::steady_clock::now();
	for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
		for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
			activateChunk(glm::ivec2(x, y));
		}
	}
	auto dur = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
	if (dur > std::chrono::microseconds{500}) {
		N++;
		if (N > 0) {
			sum += dur;
			std::cout << std::chrono::duration_cast<std::chrono::microseconds>(sum / N).count() << "\n\n";
		}
	}
}

glm::ivec2 ChunkManager::chunkPosToTexturePos(glm::ivec2 posCh) const {
	return floor_div(posCh, activeChunksRect()).rem * CHUNK_SIZE;
}

glm::ivec2 ChunkManager::chunkPosToActiveChunkPos(glm::ivec2 posCh) const {
	return floor_div(posCh, activeChunksRect()).rem;
}

void ChunkManager::activateChunk(const glm::ivec2& posCh) {
	//Check if it is not already active
	auto activePosCh = chunkPosToActiveChunkPos(posCh);
	auto& chunk = m_activeChunks[activePosCh.y * activeChunksRect().x + activePosCh.x];
	if (chunk == posCh) {
		return;//The chunk is already active
	} else {
		deactivateChunk(posCh);//Deactivate the previous chunk
		chunk = posCh;//Se the new chunk to occupy the slot
	}

	//Try to find it among inactive chunks
	auto it = m_inactiveChunks.find(posCh);
	if (it != m_inactiveChunks.end()) {
		uploadChunk(it->second.data(), posCh);
		m_inactiveChunks.erase(it);//Remove the chunk from inactive chunks
		return;//The chunk was activated from inactive chunks
	}

	try {//Try to load the chunk from file
		std::vector<unsigned char> tiles = ChunkLoader::loadChunk(m_folderPath, posCh, CHUNK_SIZE);
		//No exception was thrown, chunk has been loaded
		uploadChunk(tiles, posCh);
		return;
	}
	catch (...) {
		//Chunk is not on the disk, it has to be generated
		m_chunkGen.generateChunk(posCh, m_ws->getTexture(), chunkPosToTexturePos(posCh));
		return;
	}
}

void ChunkManager::deactivateChunk(const glm::ivec2& posCh) {
	auto activePosCh = chunkPosToActiveChunkPos(posCh);
	auto& chunk = m_activeChunks[activePosCh.y * activeChunksRect().x + activePosCh.x];
	if (chunk != NO_ACTIVE_CHUNK) {
		auto tiles = downloadChunk(activePosCh * CHUNK_SIZE);
		m_inactiveChunks.emplace(chunk, Chunk(chunk, std::move(tiles)));
		chunk = NO_ACTIVE_CHUNK;
	}
}

std::vector<unsigned char> ChunkManager::downloadChunk(const glm::ivec2& activePosTi) const {
	std::vector<unsigned char> tiles;
	tiles.resize(CHUNK_SIZE.x * CHUNK_SIZE.y * 4);
	m_ws->setTarget();
	//Data download -> CPU stall -> Pixel Buffer Object TODO
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
