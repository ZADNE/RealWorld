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
	 * Saves and frees all previous chunks.
	 *
	 * @param seed Seed of the world.
	 * @param chunkDims Dimensions of chunks in the world, in blocks
	 * @param activeChunksRect Dimensions of the active chunks rectangle, in chunks
	 * @param folderPath Path to folder containing the world.
	 * @param ws World surface of the world.
	 */
	void setTarget(int seed, glm::uvec2 chunkDims, glm::uvec2 activeChunksRect, std::string folderPath, RE::Surface* ws);

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
	 * @brief Gets the number of chunks held in memory.
	 *
	 * @return The number of chunks held in memory.
	 */
	int getNumberOfChunksLoaded();

	//Tile getters
	uchar get(TILE_VALUE type, const glm::ivec2& posTi);
	uchar getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);
	uchar getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);
	//Tile setters
	void set(TILE_VALUE type, const glm::ivec2& posTi, uchar index);

	bool exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi, uchar index);
	bool exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi, uchar index);

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
	Chunk*& getActiveChunk(glm::ivec2 posCh);

	/**
	 * @brief Activates the chunk at given position.
	 * Previous chunk at the position is deactivated.
	 * @param chunk The chunk to be activated. Must not be nullptr
	 * @param posCh Position of the chunk
	*/
	void activateChunkAtPos(Chunk* chunk, glm::ivec2 posCh, bool uploadRequired);

	/**
	 * @brief Deactivates the chunk at given position.
	 * Does nothing if there is no active chunk at the position.
	 * @param posCh Position of the chunk
	*/
	void deactivateChunkAtPos(glm::ivec2 posCh);

	/**
	 * @brief Downloads data from texture to the chunk's data
	 * @param chunk Chunk to receive the data
	 * @param posCh Position of the chunk
	*/
	void downloadChunk(Chunk* chunk, glm::ivec2 posCh) const;

	/**
	 * @brief Uploads data from chunk to texture
	 * @param chunk Chunk to upload the data from
	 * @param posCh Position of the chunk
	*/
	void uploadChunk(Chunk* chunk, glm::ivec2 posCh) const;

	/**
	 * @brief Gets chunk at given position
	 *
	 * The chunk is loaded from respective file or generated if required.
	 *
	 *
	 * @param posCh Position of the chunk
	 * @return Activated chunk
	*/
	Chunk* getChunk(glm::ivec2 posCh);
	uchar getUnsafe(TILE_VALUE type, const glm::ivec2& posTi);
	void saveChunk(Chunk& chunk, glm::ivec2 posCh) const;

	mutable std::unordered_map<glm::ivec2, Chunk> m_chunks;
	glm::ivec2 m_chunkDims = glm::ivec2(0, 0);
	glm::ivec2 m_activeChunksRect;
	std::vector<Chunk*> m_activeChunks;

	std::string m_folderPath;
	ulong m_chunkRemovalThreshold;
	ChunkGenerator m_chunkGen;
	RE::Surface* m_ws = nullptr;
	glm::ivec2 m_wsSize;
};