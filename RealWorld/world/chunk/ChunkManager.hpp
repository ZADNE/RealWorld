#pragma once
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <RealEngine/graphics/Surface.hpp>

#include <RealWorld/world/chunk/Chunk.hpp>
#include <RealWorld/world/chunk/ChunkGenerator.hpp>

/**
 * @brief Manages a collection of chunks.
 *
 * Allows access to world as if it was a single array.
 */
class ChunkManager {
public:
	/**
	 * @brief Contructs ChunkManager with default removal threshold of one minute.
	 */
	ChunkManager();

	/**
	 * @brief Calls saveAndFreeAllChunks() and deconstructs the object.
	 */
	~ChunkManager();

	/**
	 * @brief Retargets chunk handler to a new world.
	 *
	 * Saves and frees all chunks of the previous world.
	 *
	 * @param seed Seed of the new world.
	 * @param activeChunksRect Dimensions of the wrold's active chunks rectangle, in chunks
	 * @param folderPath Path to folder containing the world.
	 * @param ws World surface of the world.
	 */
	void setTarget(int seed, glm::uvec2 activeChunksRect, std::string folderPath, RE::Surface* ws);

	/**
	 * @brief Frees memory of all chunks, does not save them.
	 */
	void flushChunks();

	/**
	 * @brief Saves all chunks, keeps them in the memory.
	 *
	 * @return True if successful, false otherwise.
	 */
	bool saveChunks() const;

	/**
	 * @brief Gets the number of inactive chunks held in memory.
	 *
	 * @return The number of chunks held in memory.
	 */
	size_t getNumberOfInactiveChunks();

	/**
	 * @brief Performs beginStep operation on the chunk handler.
	 *
	 * This function increments internal timers for possible memory deallocation.
	 * It should be called every physics beginStep.
	*/
	void step();

	/**
	 * @brief Forces activation of chunks that overlap given rectangular area
	 * @param botLeftTi Bottom left corner of the rectangular area
	 * @param topRightTi Top right corner of the rectangular area
	*/
	void forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);
private:
	glm::ivec2 chunkPosToTexturePos(glm::ivec2 posCh) const;
	glm::ivec2 chunkPosToActiveChunkPos(glm::ivec2 posCh) const;

	/**
	 * @brief Activates the chunk at given position.
	 * Previous chunk at the position is deactivated.
	 * @param chunk The chunk to be activated. Must not be nullptr
	 * @param posCh Position of the chunk
	*/
	void activateChunk(const glm::ivec2& posCh);

	/**
	 * @brief Deactivates the chunk at given position.
	 *
	 * Deactivated chunk is placed to inactive chunks storage.
	 * Does nothing if there is no active chunk at the position.
	 * @param posCh Position of the chunk, measured in chunks
	*/
	void deactivateChunk(const glm::ivec2& posCh);

	/**
	 * @brief Downloads a chunk from world texture to CPU memory
	 * @param activePosTi In-texture position of the chunk, measured in tiles/pixels
	 * @return The tiles of the chunk
	*/
	std::vector<unsigned char> downloadChunk(const glm::ivec2& activePosTi) const;

	/**
	 * @brief Uploads tiles from a chunk to the world texture
	 * @param chunk Tiles to upload
	 * @param posCh Position of the chunk
	*/
	void uploadChunk(const std::vector<unsigned char>& chunk, glm::ivec2 posCh) const;

	void saveChunk(Chunk& chunk, glm::ivec2 posCh) const;

	mutable std::unordered_map<glm::ivec2, Chunk> m_inactiveChunks;
	glm::ivec2 m_activeChunksRect{0, 0};
	std::vector<glm::ivec2> m_activeChunks;
	static inline const glm::ivec2 NO_ACTIVE_CHUNK = glm::ivec2(std::numeric_limits<decltype(glm::ivec2::x)>::max());

	std::string m_folderPath;
	ulong m_chunkRemovalThreshold;
	ChunkGenerator m_chunkGen;
	RE::Surface* m_ws = nullptr;
};