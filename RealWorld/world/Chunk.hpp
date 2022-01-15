#pragma once
#include <iostream>
#include <vector>
#include <exception>

#include <glm/vec2.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"

using uchar = unsigned char;
using ushort = unsigned short;
using ulong = unsigned long;


namespace chunk {
	enum class BLOCK_VALUES : ulong {
		BLOCK = 0, BLOCK_VAR = 1, WALL = 2, WALL_VAR = 3
	};

	enum class SET_TYPES : ulong {
		BLOCK = 0, /*BLOCK_VAR = 1,*/ WALL = 2/*, WALL_VAR = 3*/
	};
}

/**
 * Class representing a rectangular area of blocks.
 */
class Chunk{
public:
	/**
	 * Constructs empty chunk.
	 */
	Chunk() :
		m_chunkPos({ 0, 0 }), m_dims({0u, 0u}) {
		
	};

	/**
	 * Contructs new chunk.
	 * 
	 * \param chunkPos Position of the chunk, in chunk coordinates.
	 * \param dims Dimensions of the chunk, in blocks.
	 * \param data Raw data of the chunk, size should be dims.x * dims.y * 4
	 * \throws std::exception If data does not hold enough bytes.
	 */
	Chunk(glm::ivec2 chunkPos, glm::uvec2 dims, std::vector<unsigned char> data):
		m_chunkPos(chunkPos), m_dims(dims), m_data(data) {
		if (data.size() < ((size_t)dims.x * dims.y * 4ull)) {
			throw std::exception();
		}
	};

	/**
	 * Deallocates all memory related to the chunk.
	 */
	~Chunk() {

	};
	
	/**
	 * Gets a value relating to a block from the chunk, checks the bounds of the chunk.
	 * Resets the timer since last operation.
	 * 
	 * \param type What type of value is requested.
	 * \param posBc Position of the block, relative to the chunk.
	 * \return Value of the block.
	 * \throws std::out_of_range When position is outside of this chunk.
	 */
	uchar inline get(chunk::BLOCK_VALUES type, glm::uvec2 posBc) const {
		m_stepsSinceLastOperation = 0;
		boundsCheck(posBc);
		return getUnsafe(type, posBc);
	};
	
	/**
	 * Gets a value relating to a block from the chunk, does not check the bounds of the chunk.
	 * Does not reset the timer since last operation.
	 * 
	 * \param type What type of value is requested.
	 * \param posBc Position of the block, relative to the chunk.
	 * \return Value of the block.
	 */
	uchar inline getUnsafe(chunk::BLOCK_VALUES type, glm::uvec2 posBc) const {
		return m_data[getIndexToBuffer(type, posBc)];
	};
	
	/**
	 * Sets a value relating to a block from the chunk, checks the bounds of the chunk.
	 * Resets the timer since last operation.
	 * 
	 * \param type What type of value is to be set.
	 * \param posBc Position of the block, relative to the chunk.
	 * \param value Value to be set.
	 * \throws std::out_of_range When position is outside of this chunk.
	 */
	void inline set(chunk::BLOCK_VALUES type, glm::uvec2 posBc, uchar value) {
		boundsCheck(posBc);
		m_stepsSinceLastOperation = 0;
		setUnsafe(type, posBc, value);
	};

	/**
	 * Sets a value relating to a block from the chunk, does not check the bounds of the chunk.
	 * Does not reset the timer since last operation.
	 * 
	 * \param type What type of value is to be set.
	 * \param posBc Position of the block, relative to the chunk.
	 * \param value Value to be set.
	 */
	void inline setUnsafe(chunk::BLOCK_VALUES type, glm::uvec2 posBc, uchar value) {
		m_data[getIndexToBuffer(type, posBc)] = value;
	};

	/**
	 * If the chunk is not active, the timer steps since last operation is incremented.
	 * If the chunk is active, timer is not incremented.
	 * Chunk is expected to be removed after a certain period of time with no operations.
	 * 
	 * \return The number of steps since last operation
	 */
	ulong step() const {
		return m_active ? m_stepsSinceLastOperation : ++m_stepsSinceLastOperation;
	};

	/**
	 * Returns raw data in vector.
	 * 
	 * \return Raw data in vector.
	 */
	std::vector<unsigned char>& data() {
		return m_data;
	}

	/**
	 * Sets chunk to be or not to be active.
	*/
	void setActive(bool active) {
		m_active = active;
		if (m_active) {
			m_stepsSinceLastOperation = 0ul;
		}
	}

	/**
	 * Tells whether the chunk is active.
	 * \return True if it is active, false otherwise.
	*/
	bool isActive() const {
		return m_active;
	}
private:
	/**
	 * Checks whether given position is within bound of the chunk.
	 * Throws when it is not.
	 * 
	 * \param posBc The position to check.
	 * \throws std::out_of_range When position is outside of this chunk.
	 */
	void inline boundsCheck(glm::uvec2 posBc) const {
		if (posBc.x >= m_dims.x || posBc.y >= m_dims.y) {
			throw std::out_of_range("Tried to get tile " + glm::to_string(posBc) + " which is outside of the requested chunk: " + glm::to_string(m_dims));
		}
	}

	/**
	 * Calculates index of value inside buffer based on position and block value type.
	 * Calculation is agnostic of chunk bounds.
	 * 
	 * \param type Block value
	 * \param posBc Position
	 * \return Index to the buffer.
	 */
	size_t inline getIndexToBuffer(chunk::BLOCK_VALUES type, glm::uvec2 posBc) const {
		return (posBc.x + ((size_t)m_dims.y - posBc.y - 1ull) * m_dims.x) * 4ull + (ulong)type;
	}

	/**
	 * Raw data of the chunk.
	 */
	std::vector<unsigned char> m_data;

	/**
	 * Position of the chunk, in chunk coordinates.
	 */
	glm::ivec2 m_chunkPos;

	/**
	 * Dimensions of the chunk, in blocks.
	 */
	glm::uvec2 m_dims;

	/**
	 * Steps since last operation on the chunk.
	 */
	mutable ulong m_stepsSinceLastOperation = 0ul;

	/**
	 * Active means that its data should be in the world texture.
	 * The actual data upload/download is not done by the chunk.
	*/
	bool m_active = true;
};