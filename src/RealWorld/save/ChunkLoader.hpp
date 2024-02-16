/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include <RealWorld/world/Chunk.hpp>

namespace rw {

/**
 * @brief Saves and loads chunks (tiles and branches)
 * @see WorldDataLoader saves and loads other world data.
 */
class ChunkLoader {
public:
    /**
     * @brief Loads chunk from file in standard location.
     *
     * @param folderPath Path to the world folder, including the file separator
     * @param posCh Position of the chunk, measured in chunks
     * @return The chunk if successfuly, empty optional otherwise
     */
    static std::optional<Chunk> loadChunk(
        const std::string& folderPath, glm::ivec2 posCh
    );

    /**
     * @brief Saves chunk to file in standard location.
     *
     * @param folderPath Path to the world folder, including the file separator
     * @throws std::runtime_error unable to encode bytes or save file
     */
    static void saveChunk(
        const std::string&       folderPath,
        glm::ivec2               posCh,
        const uint8_t*           tiles,
        std::span<const uint8_t> branchesSerialized
    );

private:
    static std::string chunkToChunkFilename(glm::ivec2 chunkPos);
};

} // namespace rw