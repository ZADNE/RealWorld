#include <RealWorld/world/Chunk.hpp>

Chunk::Chunk(glm::ivec2 chunkPosCh, glm::uvec2 dimsTi, std::vector<unsigned char> data) :
	m_chunkPosCh(chunkPosCh), m_dimsTi(dimsTi), m_data(data) {
	if (data.size() < ((size_t)dimsTi.x * dimsTi.y * 4ull)) {
		throw std::exception();
	}
}

uchar Chunk::get(chunk::BLOCK_VALUES type, glm::uvec2 posTi) const {
	m_stepsSinceLastOperation = 0;
	boundsCheck(posTi);
	return getUnsafe(type, posTi);
}

uchar Chunk::getUnsafe(chunk::BLOCK_VALUES type, glm::uvec2 posTi) const {
	return m_data[getIndexToBuffer(type, posTi)];
}

void Chunk::set(chunk::BLOCK_VALUES type, glm::uvec2 posTi, uchar value) {
	boundsCheck(posTi);
	m_stepsSinceLastOperation = 0;
	setUnsafe(type, posTi, value);
}

void Chunk::setUnsafe(chunk::BLOCK_VALUES type, glm::uvec2 posTi, uchar value) {
	m_data[getIndexToBuffer(type, posTi)] = value;
}

ulong Chunk::step() const {
	return m_active ? m_stepsSinceLastOperation : ++m_stepsSinceLastOperation;
}

std::vector<unsigned char>& Chunk::data() {
	return m_data;
}

void Chunk::setActive(bool active) {
	m_active = active;
	if (active) {
		m_stepsSinceLastOperation = 0ul;
	}
}

void Chunk::boundsCheck(glm::uvec2 posTi) const {
	if (posTi.x >= m_dimsTi.x || posTi.y >= m_dimsTi.y) {
		throw std::out_of_range("Tried to get tile " + glm::to_string(posTi) + " which is outside of the requested chunk: " + glm::to_string(m_dimsTi));
	}
}

size_t Chunk::getIndexToBuffer(chunk::BLOCK_VALUES type, glm::uvec2 posTi) const {
	return (posTi.x + ((size_t)m_dimsTi.y - posTi.y - 1ull) * m_dimsTi.x) * 4ull + (ulong)type;
}