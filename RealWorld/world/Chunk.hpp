/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <iostream>
#include <vector>
#include <exception>

#include <glm/vec2.hpp>

#include <RealWorld/constants/chunk.hpp>


/**
 * @brief Represents a fixed-size square grid of tiles.
 *
 * All chunks have the size of ChunkTi constant.
 *
 * A tile is defined by 4 values: block type, block variant, wall type and wall variant.
*/
class Chunk {
public:

    /**
     * @brief Constructs empty chunk.
     */
    Chunk() {}

    /**
     * @brief Contructs chunk from raw bytes
     *
     * @param chunkPosCh Position of the chunk, in chunk coordinates.
     * @param tiles Tiles of the chunk, size must be k_chunkByteSize
     * @throws std::exception If data does not hold enough bytes.
     */
    Chunk(const glm::ivec2& chunkPosCh, const uint8_t* tiles);

    /**
     * @brief Constructs chunk by moving tiles in
    */
    Chunk(const glm::ivec2& chunkPosCh, std::vector<uint8_t>&& tiles);

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
    uint8_t get(TileAttrib type, const glm::uvec2& posTi) const;

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
    uint8_t getUnsafe(TileAttrib type, const glm::uvec2& posTi) const;

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
    void set(TileAttrib type, const glm::uvec2& posTi, uint8_t value);

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
    void setUnsafe(TileAttrib type, const glm::uvec2& posTi, uint8_t value);

    /**
     * @brief Performs step on the chunk.
     *
     * If the chunk is not active, the timer steps since last operation is incremented.
     * If the chunk is active, timer is not incremented.
     * Chunk is expected to be removed after a certain period of time with no read/write operations.
     *
     * @return The number of steps since the last read/write operation
     */
    int step() const;

    /**
     * @brief Gets tiles of the chunk.
     *
     * @return Tiles inside a vector.
     */
    const std::vector<uint8_t>& tiles() const;

private:

    /**
     * @brief Checks whether given position is within bound of the chunk.
     *
     * Throws when it is not.
     *
     * @param posTi The position to check.
     * @throws std::out_of_range When position is outside of this chunk.
     */
    void boundsCheck(const glm::uvec2& posTi) const;

    /**
     * @brief Calculates the index of a value of a tile to the internal vector.
     *
     * The calculation is agnostic of chunk dimensions.
     *
     * @param type tile value
     * @param posTi Position
     * @return Index to the buffer.
     */
    size_t calcIndexToBuffer(TileAttrib type, const glm::uvec2& posTi) const;

    std::vector<uint8_t> m_tiles;                   /**< Tiles of the chunk */
    glm::ivec2 m_chunkPosCh{0, 0};                  /**< Position of the chunk, measured in chunk coordinates */
    mutable int m_stepsSinceLastOperation = 0;      /**< Steps since last read/write operation inside this chunk */
};