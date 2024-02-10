﻿/*!
 *  @author    Dubsky Tomas
 */
#include <stdexcept>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <RealWorld/vegetation/BranchSB.hpp>
#include <RealWorld/world/Chunk.hpp>

namespace rw {

Chunk::Chunk(
    glm::ivec2 posCh, const uint8_t* tiles, const uint8_t* branchesSerialized, size_t branchCount
)
    : Chunk(
          posCh,
          std::vector<uint8_t>{tiles, tiles + k_chunkByteSize},
          std::vector<uint8_t>{
              branchesSerialized,
              branchesSerialized + (branchCount * sizeof(BranchSerialized))}
      ) {
}

Chunk::Chunk(
    glm::ivec2 posCh, std::vector<uint8_t>&& tiles, std::vector<uint8_t>&& branchesSerialized
)
    : m_posCh(posCh)
    , m_tiles(std::move(tiles))
    , m_branchesSerialized(std::move(branchesSerialized)) {
    assert(m_tiles.size() == k_chunkByteSize);
    assert((m_branchesSerialized.size() % sizeof(BranchSerialized)) == 0);
}

uint8_t Chunk::get(TileAttrib type, glm::uvec2 posTi) const {
    m_stepsSinceLastOperation = 0;
    boundsCheck(posTi);
    return getUnsafe(type, posTi);
}

uint8_t Chunk::getUnsafe(TileAttrib type, glm::uvec2 posTi) const {
    return m_tiles[calcIndexToBuffer(type, posTi)];
}

void Chunk::set(TileAttrib type, glm::uvec2 posTi, uint8_t value) {
    boundsCheck(posTi);
    m_stepsSinceLastOperation = 0;
    setUnsafe(type, posTi, value);
}

void Chunk::setUnsafe(TileAttrib type, glm::uvec2 posTi, uint8_t value) {
    m_tiles[calcIndexToBuffer(type, posTi)] = value;
}

int Chunk::step() const {
    return ++m_stepsSinceLastOperation;
}

const std::vector<uint8_t>& Chunk::tiles() const {
    return m_tiles;
}

void Chunk::boundsCheck(glm::uvec2 posTi) const {
    if (posTi.x >= uChunkTi.x || posTi.y >= uChunkTi.y) {
        throw std::out_of_range(
            "Tried to get tile " + glm::to_string(posTi) +
            " which is outside of the requested chunk: " + glm::to_string(m_posCh)
        );
    }
}

size_t Chunk::calcIndexToBuffer(TileAttrib type, glm::uvec2 posTi) const {
    return (static_cast<size_t>(posTi.y) * iChunkTi.x + posTi.x) * 4ull +
           static_cast<size_t>(type);
}

} // namespace rw
