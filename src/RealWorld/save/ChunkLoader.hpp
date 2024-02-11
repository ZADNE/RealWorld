/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <span>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

namespace rw {

/**
 * @brief Saves and loads chunks (tiles and branches)
 * @see WorldDataLoader saves and loads other world data.
 */
class ChunkLoader {
public:
    struct ChunkData {
        std::vector<uint8_t> tiles;
        std::vector<uint8_t> branchesSerialized;
    };

    /**
     * @brief Loads chunk from file in standard location.
     *
     * @param folderPath Path to the world folder, including the file separator
     * @param chunkPos Position of the chunk, measured in chunks
     * @param chunkDims Expected dimension of the chunk
     * @return  Tiles and serialized branches of the chunk if successful
     *          Empty ChunkData if the file was not found or was corrupted
     */
    static ChunkData loadChunk(
        const std::string& folderPath, glm::ivec2 chunkPos, glm::uvec2 chunkDims
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
        const std::string&       folderPath,
        glm::ivec2               chunkPos,
        glm::uvec2               chunkDims,
        const uint8_t*           tiles,
        std::span<const uint8_t> branchesSerialized
    );

private:
    static std::string chunkToChunkFilename(glm::ivec2 chunkPos);
};

} // namespace rw