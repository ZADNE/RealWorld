#pragma once
#include <vector>
#include <string>

#include <glm/vec2.hpp>

/**
 * Static class used for both saving and loading chunks (not world data).
 * \see WorldDataLoader for saving/loading world data.
 */
class ChunkLoader{
public:
	/**
	 * Loads chunk from file in standard location.
	 * 
	 * \param folderPath Path to the world folder, including the file separator
	 * \param chunkPos Position of the chunk, measured in chunks
	 * \param chunkDims Expected dimension of the chunk
	 * \return Raw data of the chunk
	 * \throws std::runtime_error expected dimensions do not match actual dimensions
	 * \throws std::runtime_error chunk file not found or corrupted
	 */
	static std::vector<unsigned char> loadChunk(const std::string& folderPath, glm::ivec2 chunkPos, glm::uvec2 chunkDims);

	/**
	 * Saves chunk to file in standard location.
	 * 
	 * \param folderPath Path to the world folder, including the file separator
	 * \param chunkPos Position of the chunk, measured in chunks
	 * \param chunkDims Dimension of the chunk
	 * \param data Raw data of the chunk
	 * \throws std::runtime_error unable to encode bytes or save file
	 */
	static void saveChunk(const std::string& folderPath, glm::ivec2 chunkPos, glm::uvec2 chunkDims, const std::vector<unsigned char>& data);

private:
	static std::string chunkToChunkFilename(glm::ivec2 chunkPos);
};