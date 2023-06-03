/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

/**
 * @brief Saves and loads chunks.
 * @see WorldDataLoader saves and loads other world data.
 */
class ChunkLoader {
public:
    /**
     * @brief Loads chunk from file in standard location.
     *
     * @param folderPath Path to the world folder, including the file separator
     * @param chunkPos Position of the chunk, measured in chunks
     * @param chunkDims Expected dimension of the chunk
     * @return  Tiles of the chunk if successful
     *          Empty optional if the file was not found or was corrupted
     */
    static std::optional<std::vector<unsigned char>> loadChunk(
        const std::string& folderPath,
        const glm::ivec2&  chunkPos,
        const glm::uvec2&  chunkDims
    );

    /**
     * @brief Saves chunk to file in standard location.
     *
     * @param folderPath Path to the world folder, including the file separator
     * @param chunkPos Position of the chunk, measured in chunks
     * @param chunkDims Dimension of the chunk
     * @param tiles Tiles of the chunk
     * @throws std::runtime_error unable to encode bytes or save file
     */
    static void saveChunk(
        const std::string& folderPath,
        const glm::ivec2&  chunkPos,
        const glm::uvec2&  chunkDims,
        const uint8_t*     tiles
    );

private:
    static std::string chunkToChunkFilename(glm::ivec2 chunkPos);
};