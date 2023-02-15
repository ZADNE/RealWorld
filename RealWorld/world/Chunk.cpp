/*!
 *  @author    Dubsky Tomas
*/
#include <RealWorld/world/Chunk.hpp>

Chunk::Chunk(const glm::ivec2& chunkPosCh, const uint8_t* tiles):
    Chunk(chunkPosCh, std::vector<uint8_t>{tiles, tiles + CHUNK_BYTE_SIZE}) {
}

Chunk::Chunk(const glm::ivec2& chunkPosCh, std::vector<uint8_t>&& tiles):
    m_chunkPosCh(chunkPosCh), m_tiles(tiles) {
    assert(m_tiles.size() >= CHUNK_BYTE_SIZE);
}

uint8_t Chunk::get(TILE_VALUE type, const glm::uvec2& posTi) const {
    m_stepsSinceLastOperation = 0;
    boundsCheck(posTi);
    return getUnsafe(type, posTi);
}

uint8_t Chunk::getUnsafe(TILE_VALUE type, const glm::uvec2& posTi) const {
    return m_tiles[getIndexToBuffer(type, posTi)];
}

void Chunk::set(TILE_VALUE type, const glm::uvec2& posTi, uint8_t value) {
    boundsCheck(posTi);
    m_stepsSinceLastOperation = 0;
    setUnsafe(type, posTi, value);
}

void Chunk::setUnsafe(TILE_VALUE type, const glm::uvec2& posTi, uint8_t value) {
    m_tiles[getIndexToBuffer(type, posTi)] = value;
}

int Chunk::step() const {
    return ++m_stepsSinceLastOperation;
}

const std::vector<uint8_t>& Chunk::tiles() const {
    return m_tiles;
}

void Chunk::boundsCheck(const glm::uvec2& posTi) const {
    if (posTi.x >= static_cast<unsigned int>(iCHUNK_DIMS.x) || posTi.y >= static_cast<unsigned int>(iCHUNK_DIMS.y)) {
        throw std::out_of_range("Tried to get tile " + glm::to_string(posTi) + " which is outside of the requested chunk: " + glm::to_string(m_chunkPosCh));
    }
}

size_t Chunk::getIndexToBuffer(TILE_VALUE type, const glm::uvec2& posTi) const {
    return (static_cast<size_t>(posTi.y) * iCHUNK_DIMS.x + posTi.x) * 4ull + static_cast<size_t>(type);
}
