/**
 *  @author    Dubsky Tomas
 */
#include <RealWorld/simulation/tiles/Chunk.hpp>
#include <RealWorld/simulation/vegetation/BranchSerialized.hpp>

namespace rw {

Chunk::Chunk(glm::ivec2 posCh, const uint8_t* tiles, std::span<const uint8_t> branchesSerialized)
    : Chunk(
          posCh, std::vector<uint8_t>{tiles, tiles + k_chunkByteSize},
          std::vector<uint8_t>{branchesSerialized.begin(), branchesSerialized.end()}
      ) {
}

Chunk::Chunk(
    glm::ivec2 posCh, std::vector<uint8_t>&& tiles,
    std::vector<uint8_t>&& branchesSerialized
)
    : m_posCh(posCh)
    , m_tiles(std::move(tiles))
    , m_branchesSerialized(std::move(branchesSerialized)) {
    assert(m_tiles.size() == k_chunkByteSize);
    assert((m_branchesSerialized.size() % sizeof(BranchSerialized)) == 0);
}

int Chunk::step() const {
    return ++m_stepsSinceLastOperation;
}

} // namespace rw
