/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <exception>
#include <span>
#include <vector>

#include <glm/vec2.hpp>

#include <RealWorld/constants/chunk.hpp>

namespace rw {

/**
 * @brief Represents a square of tiles and vegetation belonging to it.
 *
 * All chunks have the dimensions of ChunkTi constant. The number of branches
 * per chunk varies.
 *
 * A tile is defined by 4 values: block type, block variant, wall type and wall
 * variant. All 4 values are 16 bits.
 */
class Chunk {
public:
    /**
     * @brief Contructs chunk from raw bytes
     */
    Chunk(
        glm::ivec2 posCh, const uint8_t* tiles,
        std::span<const uint8_t> branchesSerialized
    );

    /**
     * @brief Constructs chunk by moving tiles in
     */
    Chunk(
        glm::ivec2 posCh, std::vector<uint8_t>&& tiles,
        std::vector<uint8_t>&& branchesSerialized
    );

    glm::ivec2 posCh() const { return m_posCh; }

    /**
     * @brief Performs step on the chunk.
     *
     * If the chunk is not active, the timer steps since last operation is
     * incremented. If the chunk is active, timer is not incremented. Chunk is
     * expected to be removed after a certain period of time with no read/write
     * operations.
     *
     * @return The number of steps since the last read/write operation
     */
    int step() const;

    /**
     * @brief Gets tiles of the chunk
     */
    [[nodiscard]] const std::vector<uint8_t>& tiles() const { return m_tiles; }

    /**
     * @brief Gets serialized branches of the chunk
     */
    [[nodiscard]] const std::vector<uint8_t>& branchesSerialized() const {
        return m_branchesSerialized;
    }

private:
    std::vector<uint8_t> m_tiles; ///< First block layer, then wall layer
    std::vector<uint8_t> m_branchesSerialized;
    glm::ivec2 m_posCh{0, 0};
    mutable int m_stepsSinceLastOperation = 0; ///< Steps since last read/write op
};

} // namespace rw
