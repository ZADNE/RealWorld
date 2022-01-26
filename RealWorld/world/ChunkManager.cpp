#include <RealWorld/world/ChunkManager.hpp>

#include <iostream>
#include <algorithm>

#include <SDL2/SDL_timer.h>

#include <RealWorld/metadata.hpp>
#include <RealWorld/world/ChunkLoader.hpp>
#include <RealWorld/div.hpp>



ChunkManager::ChunkManager() :
	m_chunkRemovalThreshold(PHYSICS_STEPS_PER_SECOND * 60) {

}

ChunkManager::~ChunkManager() {
	flushChunks();
}

void ChunkManager::setTarget(int seed, glm::uvec2 chunkDims, glm::uvec2 activeChunksRect, std::string folderPath, RE::Surface* ws) {
	flushChunks();
	m_chunkDims = chunkDims;
	m_folderPath = folderPath;
	m_chunkGen.setTargetWorld(seed, chunkDims, activeChunksRect);
	m_ws = ws;
	m_wsSize = static_cast<glm::ivec2>(m_ws->getDims());
	m_activeChunksRect = static_cast<glm::ivec2>(activeChunksRect);
	m_activeChunks.clear();
	m_activeChunks.resize((size_t)m_activeChunksRect.x * m_activeChunksRect.y, nullptr);
}

void ChunkManager::flushChunks() {
	m_chunks.clear();
}

bool ChunkManager::saveChunks() const {
	for (auto& pair : m_chunks) {
		saveChunk(pair.second, pair.first);
	}
	return true;
}

int ChunkManager::getNumberOfChunksLoaded() {
	return (int)m_chunks.size();
}

uchar ChunkManager::get(chunk::BLOCK_VALUES type, const glm::ivec2& posBc) {
	ivec2_div_t div = floor_div(posBc, m_chunkDims);
	return getChunk(div.quot)->get(type, div.rem);
}

uchar ChunkManager::getMax(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc) {
	uchar returnValue = std::numeric_limits<uchar>::min();
	//Force load of chunks to use unsafe
	forceActivationOfChunks(botLeftBc, topRightBc);
	//Search using unsafe
	for (int j = botLeftBc.y; j <= topRightBc.y; j++) {
		for (int i = botLeftBc.x; i <= topRightBc.x; i++) {
			returnValue = std::max(returnValue, getUnsafe(type, glm::ivec2(i, j)));
		}
	}
	return returnValue;
}

uchar ChunkManager::getMin(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc) {
	uchar returnValue = std::numeric_limits<uchar>::max();
	//Force load of chunks to use unsafe
	forceActivationOfChunks(botLeftBc, topRightBc);
	//Search using unsafe
	for (int j = botLeftBc.y; j <= topRightBc.y; j++) {
		for (int i = botLeftBc.x; i <= topRightBc.x; i++) {
			returnValue = std::min(returnValue, getUnsafe(type, glm::ivec2(i, j)));
		}
	}
	return returnValue;
}

void ChunkManager::set(chunk::BLOCK_VALUES type, const glm::ivec2& posBc, uchar index) {
	ivec2_div_t div = floor_div(posBc, m_chunkDims);
	getChunk(div.quot)->set(type, div.rem, index);
}

bool ChunkManager::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc, uchar index) {
	if (get(type, botLeftBc) == index) { return true; }
	if (get(type, botLeftBc + glm::ivec2(dimBc)) == index) { return true; }
	for (int j = 0; j < (int)dimBc.y; j++) {
		for (int i = 0; i < (int)dimBc.x; i++) {
			if (getUnsafe(type, botLeftBc + glm::ivec2(i, j)) == index) {
				return true;//Index found
			}
		}
	}
	return false;
}

bool ChunkManager::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc, uchar index) {
	if (get(type, botLeftBc) == index) { return true; }
	if (get(type, topRightBc) == index) { return true; }
	for (int j = botLeftBc.y; j <= topRightBc.y; j++) {
		for (int i = botLeftBc.x; i <= topRightBc.x; i++) {
			if (getUnsafe(type, glm::ivec2(i, j)) == index) {
				return true;//Index found
			}
		}
	}
	return false;
}

void ChunkManager::step() {
	for (auto it = m_chunks.begin(); it != m_chunks.end();) {
		if (it->second.step() >= m_chunkRemovalThreshold) {
			deactivateChunkAtPos(it->first);
			saveChunk(it->second, it->first);
			it = m_chunks.erase(it);
		} else { it++; }
	}
}

void ChunkManager::forceActivationOfChunks(const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc) {
	glm::ivec2 botLeftCh = floor_div(botLeftBc, m_chunkDims).quot;
	glm::ivec2 topRightCh = floor_div(topRightBc, m_chunkDims).quot;

	for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
		for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
			getChunk(glm::ivec2(x, y));
		}
	}
}

glm::ivec2 ChunkManager::chunkPosToTexturePos(glm::ivec2 posCh) const {
	glm::ivec2 origin = glm::ivec2(0, m_wsSize.y - m_chunkDims.y);
	return origin + floor_div(posCh, m_activeChunksRect).rem * m_chunkDims * glm::ivec2{1, -1};
}

glm::ivec2 ChunkManager::chunkPosToActiveChunkPos(glm::ivec2 posCh) const {
	glm::ivec2 origin = glm::ivec2(0, m_activeChunksRect.y - 1);
	return origin + floor_div(posCh, m_activeChunksRect).rem * glm::ivec2{1, -1};
}

Chunk*& ChunkManager::getActiveChunk(glm::ivec2 posCh) {
	glm::ivec2 pos = chunkPosToActiveChunkPos(posCh);
	return m_activeChunks[pos.x + pos.y * m_activeChunksRect.x];
}

void ChunkManager::activateChunkAtPos(Chunk* chunk, glm::ivec2 posCh, bool uploadRequired) {
	//First deactivate the previous chunk
	deactivateChunkAtPos(posCh);
	//Activate new chunk
	Chunk*& prev = getActiveChunk(posCh);
	prev = chunk;
	chunk->setActive(true);
	if (uploadRequired) { //Upload it, if required
		uploadChunk(chunk, posCh);
	}
}

void ChunkManager::deactivateChunkAtPos(glm::ivec2 posCh) {
	Chunk*& chunk = getActiveChunk(posCh);
	if (chunk) {
		chunk->setActive(false);
		downloadChunk(chunk, posCh);
		chunk = nullptr;
	}
}

void ChunkManager::downloadChunk(Chunk* chunk, glm::ivec2 posCh) const {
	glm::ivec2 texturePos = chunkPosToTexturePos(posCh);
	m_ws->setTarget();
	//Download data -> CPU stall -> Pixel Buffer Object todo
	glReadPixels(texturePos.x, texturePos.y, m_chunkDims.x, m_chunkDims.y, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, chunk->data().data());
	m_ws->resetTarget();
}

void ChunkManager::uploadChunk(Chunk* chunk, glm::ivec2 posCh) const {
	glm::ivec2 texturePos = chunkPosToTexturePos(posCh);
	glTextureSubImage2D(m_ws->getTextureID(), 0, texturePos.x, texturePos.y, m_chunkDims.x, m_chunkDims.y, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, chunk->data().data());
}

Chunk* ChunkManager::getChunk(glm::ivec2 posCh) {
	auto it = m_chunks.find(posCh);
	if (it != m_chunks.end()) {//Chunk is in the memory
		if (it->second.isActive()) {//Chunk is in the memory and it is active
			return &(it->second);
		} else {//Chunk is in the memory but it is not active
			activateChunkAtPos(&(it->second), posCh, true);
			return &(it->second); //Return activated chunk
		}
	}

	//Chunk is not in the memory

	try {//Try to load the chunk from file
		std::vector<unsigned char> data = ChunkLoader::loadChunk(m_folderPath, posCh, m_chunkDims);
		//No exception was thrown, chunk has been loaded
		auto pair = m_chunks.insert(std::make_pair(posCh, Chunk{posCh, m_chunkDims, std::move(data)}));
		activateChunkAtPos(&(pair.first->second), posCh, true);
		return &(pair.first->second);
	}
	catch (...) {
		//Chunk is not on the disk, it has to be generated
		deactivateChunkAtPos(posCh);//Deactivate previous chunk because the texture will get overwritten
		auto pair = m_chunks.insert(std::make_pair(posCh, m_chunkGen.generateChunk(posCh, m_ws->getTextureID(), chunkPosToTexturePos(posCh))));
		activateChunkAtPos(&(pair.first->second), posCh, true);//Generator draws to the texture upload is not needed
		return &(pair.first->second);
	}
}

uchar ChunkManager::getUnsafe(chunk::BLOCK_VALUES type, const glm::ivec2& posBc) {
	ivec2_div_t div = floor_div(posBc, m_chunkDims);
	return m_chunks[div.quot].getUnsafe(type, div.rem);
}

void ChunkManager::saveChunk(Chunk& chunk, glm::ivec2 posCh) const {
	if (chunk.isActive()) {
		//Active chunk, has to be downloaded
		downloadChunk(&chunk, posCh);
	}
	ChunkLoader::saveChunk(m_folderPath, posCh, m_chunkDims, chunk.data());
}
