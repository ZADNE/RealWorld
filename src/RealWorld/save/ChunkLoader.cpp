/*!
 *  @author    Dubsky Tomas
 */
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <lodepng/lodepng.hpp>

#include <RealWorld/save/ChunkLoader.hpp>

namespace rw {

std::vector<unsigned char> ChunkLoader::loadChunk(
    const std::string& folderPath, glm::ivec2 chunkPos, glm::uvec2 chunkDims
) {
    std::vector<unsigned char> bytes;

    glm::uvec2 realDims;

    std::string fullPath = folderPath + chunkToChunkFilename(chunkPos);

    unsigned int error = lodepng::decode(
        bytes, realDims.x, realDims.y, fullPath, LodePNGColorType::LCT_RGBA, 8u
    );
    if (error || chunkDims != realDims) {
        bytes.resize(0);
        return bytes; // Error loading the chunk
    }

    return bytes;
}

void ChunkLoader::saveChunk(
    const std::string& folderPath,
    glm::ivec2         chunkPos,
    glm::uvec2         chunkDims,
    const uint8_t*     tiles
) {
    std::string fullPath = folderPath + chunkToChunkFilename(chunkPos);

#ifndef _DEBUG
    unsigned int error = lodepng::encode(
        fullPath, tiles, chunkDims.x, chunkDims.y, LodePNGColorType::LCT_RGBA, 8u
    );

    if (error)
        throw std::runtime_error("Error encoding or saving chunk " + fullPath);
#endif // ! _DEBUG
}

std::string ChunkLoader::chunkToChunkFilename(glm::ivec2 chunkPos) {
    std::stringstream ss;
    ss << "chunk_" << (chunkPos.x >= 0 ? '+' : '-') << std::setw(4)
       << std::setfill('0') << std::abs(chunkPos.x) << 'x'
       << (chunkPos.y >= 0 ? '+' : '-') << std::setw(4) << std::setfill('0')
       << std::abs(chunkPos.y) << ".chunk";
    return ss.str();
}

} // namespace rw
