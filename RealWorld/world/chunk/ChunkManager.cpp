﻿#include <RealWorld/world/chunk/ChunkManager.hpp>

#include <iostream>
#include <algorithm>

#include <RealWorld/world/chunk/ChunkLoader.hpp>
#include <RealWorld/div.hpp>


#include <chrono>


ChunkManager::ChunkManager() :
	m_chunkRemovalThreshold(PHYSICS_STEPS_PER_SECOND * 60) {

}

ChunkManager::~ChunkManager() {
	flushChunks();
}

void ChunkManager::setTarget(int seed, glm::uvec2 activeChunksRect, std::string folderPath, RE::Surface* ws) {
	flushChunks();
	m_folderPath = folderPath;
	m_chunkGen.setSeed(seed);
	m_ws = ws;
	m_wsSize = static_cast<glm::ivec2>(m_ws->getDims());
	m_activeChunksRect = static_cast<glm::ivec2>(activeChunksRect);
	m_activeChunks.clear();
	m_activeChunks.resize(static_cast<size_t>(m_activeChunksRect.x) * m_activeChunksRect.y, nullptr);
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

size_t ChunkManager::getNumberOfChunksLoaded() {
	return m_chunks.size();
}

uchar ChunkManager::get(TILE_VALUE type, const glm::ivec2& posTi) {
	ivec2_div_t div = floor_div(posTi, CHUNK_SIZE);
	return getChunk(div.quot)->get(type, div.rem);
}

uchar ChunkManager::getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	uchar returnValue = std::numeric_limits<uchar>::min();
	//Force load of chunks to use unsafe
	forceActivationOfChunks(botLeftTi, topRightTi);
	//Search using unsafe
	for (int j = botLeftTi.y; j <= topRightTi.y; j++) {
		for (int i = botLeftTi.x; i <= topRightTi.x; i++) {
			returnValue = std::max(returnValue, getUnsafe(type, glm::ivec2(i, j)));
		}
	}
	return returnValue;
}

uchar ChunkManager::getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	uchar returnValue = std::numeric_limits<uchar>::max();
	//Force load of chunks to use unsafe
	forceActivationOfChunks(botLeftTi, topRightTi);
	//Search using unsafe
	for (int j = botLeftTi.y; j <= topRightTi.y; j++) {
		for (int i = botLeftTi.x; i <= topRightTi.x; i++) {
			returnValue = std::min(returnValue, getUnsafe(type, glm::ivec2(i, j)));
		}
	}
	return returnValue;
}

void ChunkManager::set(TILE_VALUE type, const glm::ivec2& posTi, uchar index) {
	ivec2_div_t div = floor_div(posTi, CHUNK_SIZE);
	getChunk(div.quot)->set(type, div.rem, index);
}

bool ChunkManager::exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi, uchar index) {
	if (get(type, botLeftTi) == index) { return true; }
	if (get(type, botLeftTi + glm::ivec2(dimsTi)) == index) { return true; }
	for (int j = 0; j < (int)dimsTi.y; j++) {
		for (int i = 0; i < (int)dimsTi.x; i++) {
			if (getUnsafe(type, botLeftTi + glm::ivec2(i, j)) == index) {
				return true;//Index found
			}
		}
	}
	return false;
}

bool ChunkManager::exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi, uchar index) {
	if (get(type, botLeftTi) == index) { return true; }
	if (get(type, topRightTi) == index) { return true; }
	for (int j = botLeftTi.y; j <= topRightTi.y; j++) {
		for (int i = botLeftTi.x; i <= topRightTi.x; i++) {
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

void ChunkManager::forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	glm::ivec2 botLeftCh = floor_div(botLeftTi, CHUNK_SIZE).quot;
	glm::ivec2 topRightCh = floor_div(topRightTi, CHUNK_SIZE).quot;

	static int N = -7;
	static std::chrono::nanoseconds sum = std::chrono::nanoseconds::zero();
	auto start = std::chrono::steady_clock::now();
	for (int x = botLeftCh.x; x <= topRightCh.x; ++x) {
		for (int y = botLeftCh.y; y <= topRightCh.y; ++y) {
			getChunk(glm::ivec2(x, y));
		}
	}
	auto dur = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
	if (dur > std::chrono::microseconds{500}) {
		N++;
		if (N > 0) {
			sum += dur;
			std::cout << std::chrono::duration_cast<std::chrono::microseconds>(sum / N) << "\n\n";
		}
	}
}

glm::ivec2 ChunkManager::chunkPosToTexturePos(glm::ivec2 posCh) const {
	return floor_div(posCh, m_activeChunksRect).rem * CHUNK_SIZE;
}

glm::ivec2 ChunkManager::chunkPosToActiveChunkPos(glm::ivec2 posCh) const {
	return floor_div(posCh, m_activeChunksRect).rem;
}

Chunk*& ChunkManager::getActiveChunk(glm::ivec2 posCh) {
	glm::ivec2 pos = chunkPosToActiveChunkPos(posCh);
	return m_activeChunks[static_cast<size_t>(pos.y) * m_activeChunksRect.x + pos.x];
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
	glReadnPixels(texturePos.x, texturePos.y, CHUNK_SIZE.x, CHUNK_SIZE.y, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, static_cast<GLsizei>(chunk->data().size()), chunk->data().data());
	m_ws->resetTarget();
}

void ChunkManager::uploadChunk(Chunk* chunk, glm::ivec2 posCh) const {
	glm::ivec2 texturePos = chunkPosToTexturePos(posCh);
	m_ws->getTexture().setTexelsWithinImage(0, texturePos, CHUNK_SIZE, chunk->data().data());
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
		std::vector<unsigned char> data = ChunkLoader::loadChunk(m_folderPath, posCh, CHUNK_SIZE);
		//No exception was thrown, chunk has been loaded
		auto pair = m_chunks.insert(std::make_pair(posCh, Chunk{posCh, std::move(data)}));
		activateChunkAtPos(&(pair.first->second), posCh, true);
		return &(pair.first->second);
	}
	catch (...) {
		//Chunk is not on the disk, it has to be generated
		deactivateChunkAtPos(posCh);//Deactivate previous chunk because the texture will get overwritten
		auto pair = m_chunks.insert(std::make_pair(posCh, m_chunkGen.generateChunk(posCh, m_ws->getTexture(), chunkPosToTexturePos(posCh))));
		activateChunkAtPos(&(pair.first->second), posCh, false);//Generator draws to the texture upload is not needed
		return &(pair.first->second);
	}
}

uchar ChunkManager::getUnsafe(TILE_VALUE type, const glm::ivec2& posTi) {
	ivec2_div_t div = floor_div(posTi, CHUNK_SIZE);
	return m_chunks[div.quot].getUnsafe(type, div.rem);
}

void ChunkManager::saveChunk(Chunk& chunk, glm::ivec2 posCh) const {
	if (chunk.isActive()) {
		//Active chunk, has to be downloaded
		downloadChunk(&chunk, posCh);
	}
	ChunkLoader::saveChunk(m_folderPath, posCh, CHUNK_SIZE, chunk.data());
}
