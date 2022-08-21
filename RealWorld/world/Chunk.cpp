/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/Chunk.hpp>

Chunk::Chunk(const glm::ivec2& chunkPosCh, std::vector<unsigned char> data) :
	m_chunkPosCh(chunkPosCh), m_data(data) {
	if (data.size() < (static_cast<size_t>(iCHUNK_SIZE.x) * iCHUNK_SIZE.y * 4ull)) {
		throw std::exception("Not enough bytes provided to construct a chunk!");
	}
}

uchar Chunk::get(TILE_VALUE type, const glm::uvec2& posTi) const {
	m_stepsSinceLastOperation = 0;
	boundsCheck(posTi);
	return getUnsafe(type, posTi);
}

uchar Chunk::getUnsafe(TILE_VALUE type, const glm::uvec2& posTi) const {
	return m_data[getIndexToBuffer(type, posTi)];
}

void Chunk::set(TILE_VALUE type, const glm::uvec2& posTi, uchar value) {
	boundsCheck(posTi);
	m_stepsSinceLastOperation = 0;
	setUnsafe(type, posTi, value);
}

void Chunk::setUnsafe(TILE_VALUE type, const glm::uvec2& posTi, uchar value) {
	m_data[getIndexToBuffer(type, posTi)] = value;
}

ulong Chunk::step() const {
	return ++m_stepsSinceLastOperation;
}

std::vector<unsigned char>& Chunk::data() {
	return m_data;
}

void Chunk::boundsCheck(const glm::uvec2& posTi) const {
	if (posTi.x >= static_cast<unsigned int>(iCHUNK_SIZE.x) || posTi.y >= static_cast<unsigned int>(iCHUNK_SIZE.y)) {
		throw std::out_of_range("Tried to get tile " + glm::to_string(posTi) + " which is outside of the requested chunk: " + glm::to_string(m_chunkPosCh));
	}
}

size_t Chunk::getIndexToBuffer(TILE_VALUE type, const glm::uvec2& posTi) const {
	return (static_cast<size_t>(posTi.y) * iCHUNK_SIZE.x + posTi.x) * 4ull + (ulong)type;
}