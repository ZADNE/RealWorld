/*!
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
 * @brief Represents a fixed-size square of tiles and vegetation belonging to it.
 *
 * All chunks have the size of ChunkTi constant. Vegetation per chunk varies.
 *
 * A tile is defined by 4 values: block type, block variant, wall type and wall
 * variant.
 */
class Chunk {
public:
    /**
     * @brief Contructs chunk from raw bytes
     * @param posCh Position of the chunk, in chunk coordinates.
     */
    Chunk(
        glm::ivec2 posCh, const uint8_t* tiles, std::span<const uint8_t> branchesSerialized
    );

    /**
     * @brief Constructs chunk by moving tiles in
     */
    Chunk(
        glm::ivec2 posCh, std::vector<uint8_t>&& tiles, std::vector<uint8_t>&& branchesSerialized
    );

    /**
     * @brief Gets a value of a tile inside the chunk.
     *
     * Checks whether the tile is inside the bounds of the chunk.
     * Resets the timer since last operation.
     *
     * @param type Tells which value of the tile is requested.
     * @param posTi Position of the tile, relative to this chunk.
     * @return The value of the tile.
     * @throws std::out_of_range When position is outside of this chunk.
     */
    uint8_t get(TileAttrib type, glm::uvec2 posTi) const;

    /**
     * @brief Gets a value of a tile inside the chunk.
     *
     * Does not check whether the tile is inside the bounds of the chunk.
     * Does not reset the timer since last operation.
     *
     * @param type Tells which value of the tile is requested.
     * @param posTi Position of the tile, relative to this chunk.
     * @return The value of the tile.
     */
    uint8_t getUnsafe(TileAttrib type, glm::uvec2 posTi) const;

    /**
     * @brief Sets a value of a tile inside the chunk.
     *
     * Checks the bounds of the chunk.
     * Resets the timer since last operation.
     *
     * @param type Tells which value of the tile is to be set.
     * @param posTi Position of the tile, relative to this chunk.
     * @param value The value to be set.
     * @throws std::out_of_range When position is outside of this chunk.
     */
    void set(TileAttrib type, glm::uvec2 posTi, uint8_t value);

    /**
     * @brief Sets a value of a tile inside the chunk.
     *
     * Does not check the bounds of the chunk.
     * Does not reset the timer since last operation.
     *
     * @param type Tells which value of the tile is to be set.
     * @param posTi Position of the tile, relative to this chunk.
     * @param value The value to be set.
     */
    void setUnsafe(TileAttrib type, glm::uvec2 posTi, uint8_t value);

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
    /**
     * @brief Checks whether given position is within bound of the chunk.
     *
     * Throws when it is not.
     *
     * @param posTi The position to check.
     * @throws std::out_of_range When position is outside of this chunk.
     */
    void boundsCheck(glm::uvec2 posTi) const;

    /**
     * @brief Calculates the index of a value of a tile to the internal vector.
     *
     * The calculation is agnostic of chunk dimensions.
     *
     * @param type tile value
     * @param posTi Position
     * @return Index to the buffer.
     */
    size_t calcIndexToBuffer(TileAttrib type, glm::uvec2 posTi) const;

    std::vector<uint8_t> m_tiles; /**< Tiles of the chunk */
    std::vector<uint8_t> m_branchesSerialized;
    glm::ivec2           m_posCh{0, 0};
    mutable int m_stepsSinceLastOperation = 0; /**< Steps since last read/write op */
};

} // namespace rw