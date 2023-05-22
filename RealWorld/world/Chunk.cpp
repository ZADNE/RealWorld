/*!
 *  @author    Dubsky Tomas
*/
#include <RealWorld/world/Chunk.hpp>

#include <glm/gtx/string_cast.hpp>

Chunk::Chunk(const glm::ivec2& chunkPosCh, const uint8_t* tiles):
    Chunk(chunkPosCh, std::vector<uint8_t>{tiles, tiles + k_chunkByteSize}) {
}

Chunk::Chunk(const glm::ivec2& chunkPosCh, std::vector<uint8_t>&& tiles):
    m_chunkPosCh(chunkPosCh), m_tiles(tiles) {
    assert(m_tiles.size() >= k_chunkByteSize);
}

uint8_t Chunk::get(TileAttrib type, const glm::uvec2& posTi) const {
    m_stepsSinceLastOperation = 0;
    boundsCheck(posTi);
    return getUnsafe(type, posTi);
}

uint8_t Chunk::getUnsafe(TileAttrib type, const glm::uvec2& posTi) const {
    return m_tiles[getIndexToBuffer(type, posTi)];
}

void Chunk::set(TileAttrib type, const glm::uvec2& posTi, uint8_t value) {
    boundsCheck(posTi);
    m_stepsSinceLastOperation = 0;
    setUnsafe(type, posTi, value);
}

void Chunk::setUnsafe(TileAttrib type, const glm::uvec2& posTi, uint8_t value) {
    m_tiles[getIndexToBuffer(type, posTi)] = value;
}

int Chunk::step() const {
    return ++m_stepsSinceLastOperation;
}

const std::vector<uint8_t>& Chunk::tiles() const {
    return m_tiles;
}

void Chunk::boundsCheck(const glm::uvec2& posTi) const {
    if (posTi.x >= static_cast<unsigned int>(iChunkTi.x) || posTi.y >= static_cast<unsigned int>(iChunkTi.y)) {
        throw std::out_of_range("Tried to get tile " + glm::to_string(posTi) + " which is outside of the requested chunk: " + glm::to_string(m_chunkPosCh));
    }
}

size_t Chunk::getIndexToBuffer(TileAttrib type, const glm::uvec2& posTi) const {
    return (static_cast<size_t>(posTi.y) * iChunkTi.x + posTi.x) * 4ull + static_cast<size_t>(type);
}
