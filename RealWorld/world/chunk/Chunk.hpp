#pragma once
#include <iostream>
#include <vector>
#include <exception>

#include <glm/vec2.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

using uchar = unsigned char;
using ushort = unsigned short;
using ulong = unsigned long;

/**
 * @brief Dimensions of a chunk, in tiles
 *
 * All chunks have these dimensions
*/
const glm::ivec2 CHUNK_SIZE = glm::ivec2(128, 128);


enum class TILE_VALUE : ulong {
	BLOCK = 0, BLOCK_VAR = 1, WALL = 2, WALL_VAR = 3
};

enum class SET_TYPES : ulong {
	BLOCK = 0, /*BLOCK_VAR = 1,*/ WALL = 2/*, WALL_VAR = 3*/
};

/**
 * @brief Represents a rectangular area of tiles.
 *
 * Each tile contains 4 values: block ID, block variation,
 * wall ID and wall variation.
 */
class Chunk {
public:
	/**
	 * @brief Constructs empty chunk.
	 */
	Chunk() {}

	/**
	 * @brief Contructs new chunk.
	 *
	 * @param chunkPosCh Position of the chunk, in chunk coordinates.
	 * @param data Raw data of the chunk, size should be dimsTi.x * dimsTi.y * 4
	 * @throws std::exception If data does not hold enough bytes.
	 */
	Chunk(const glm::ivec2& chunkPosCh, std::vector<unsigned char> data);

	/**
	 * @brief Destroys the chunk, frees its data.
	 */
	~Chunk() {};

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
	uchar get(TILE_VALUE type, const glm::uvec2& posTi) const;

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
	uchar getUnsafe(TILE_VALUE type, const glm::uvec2& posTi) const;

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
	void set(TILE_VALUE type, const glm::uvec2& posTi, uchar value);

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
	void setUnsafe(TILE_VALUE type, const glm::uvec2& posTi, uchar value);

	/**
	 * @brief Performs step on the chunk.
	 *
	 * If the chunk is not active, the timer steps since last operation is incremented.
	 * If the chunk is active, timer is not incremented.
	 * Chunk is expected to be removed after a certain period of time with no read/write operations.
	 *
	 * @return The number of steps since the last read/write operation
	 */
	ulong step() const;

	/**
	 * @brief Gets raw data of the chunk.
	 *
	 * @return Raw data inside a vector.
	 */
	std::vector<unsigned char>& data();

	/**
	 * @brief Sets whether the chunk is active or not.
	*/
	void setActive(bool active);

	/**
	 * @brief Tells whether the chunk is active.
	 *
	 * If it is active, it means that its data is inside the world texture.
	 * @return True if it is active, false otherwise.
	*/
	bool isActive() const { return m_active; }
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
	size_t getIndexToBuffer(TILE_VALUE type, const glm::uvec2& posTi) const;

	std::vector<unsigned char> m_data;				/**< Raw data of the chunk */
	glm::ivec2 m_chunkPosCh{0, 0};					/**< Position of the chunk, measured in chunk coordinates */
	mutable ulong m_stepsSinceLastOperation = 0ul;	/**< Steps since last read/write operation inside this chunk */

	/**
	 * If the chunk is active, it means that its data are in the world texture.
	 * The actual data upload/download is not done by the chunk.
	*/
	bool m_active = true;
};