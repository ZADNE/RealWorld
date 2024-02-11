/*!
 *  @author    Dubsky Tomas
 */
#include <array>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <lodepng/lodepng.hpp>

#include <RealEngine/utility/BuildType.hpp>

#include <RealWorld/save/ChunkLoader.hpp>

namespace rw {

using PNGChunkName = std::array<char, 5>;
constexpr PNGChunkName k_branchPNGChunkName{'b', 'r', 'N', 'c', '\0'};

ChunkLoader::ChunkData ChunkLoader::loadChunk(
    const std::string& folderPath, glm::ivec2 chunkPos, glm::uvec2 chunkDims
) {
    lodepng::State state{};
    state.decoder.remember_unknown_chunks = 1;
    state.info_png.color.colortype        = LCT_RGBA;
    state.decoder.color_convert           = 1;
    ChunkData                  out{};
    glm::uvec2                 realDims;
    unsigned int               err;
    std::vector<unsigned char> encoded;
    std::string fullPath = folderPath + chunkToChunkFilename(chunkPos);

    // Load file and decode tiles
    if ((err = lodepng::load_file(encoded, fullPath)) ||
        (err = lodepng::decode(out.tiles, realDims.x, realDims.y, state, encoded)) ||
        chunkDims != realDims) {
        // Loading file or decoding tiles failed
        return ChunkData{};
    }

    // Load branches (if present)
    for (size_t unknownChunkPos = 0; unknownChunkPos < 3; unknownChunkPos++) {
        const auto& unknownData = state.info_png.unknown_chunks_data[unknownChunkPos];
        const auto& unknownDataEnd =
            &unknownData[state.info_png.unknown_chunks_size[unknownChunkPos]];
        for (auto chunk = unknownData; chunk != unknownDataEnd;
             chunk      = lodepng_chunk_next(chunk, unknownDataEnd)) {
            PNGChunkName chunkName;
            lodepng_chunk_type(chunkName.data(), chunk);
            if (chunkName == k_branchPNGChunkName) {
                const uint8_t* data = lodepng_chunk_data_const(chunk);
                out.branchesSerialized.assign(data, data + lodepng_chunk_length(chunk));
                return out; // Successfully loaded RTI
            }
        }
    }

    return out;
}

void ChunkLoader::saveChunk(
    const std::string&       folderPath,
    glm::ivec2               chunkPos,
    glm::uvec2               chunkDims,
    const uint8_t*           tiles,
    std::span<const uint8_t> branchesSerialized
) {
    if constexpr (re::k_buildType == re::BuildType::Release) {
        lodepng::State state{};
        unsigned int   err;

        // Create chunk with branches
        if (err = lodepng_chunk_create(
                &state.info_png.unknown_chunks_data[0],
                &state.info_png.unknown_chunks_size[0],
                static_cast<unsigned int>(branchesSerialized.size_bytes()),
                k_branchPNGChunkName.data(),
                branchesSerialized.data()
            )) {
            // Chunk creation failed
            throw std::runtime_error{lodepng_error_text(err)};
        }

        // Encode tiles and save file
        std::string fullPath = folderPath + chunkToChunkFilename(chunkPos);
        state.info_png.color.colortype = LCT_RGBA;
        state.encoder.auto_convert     = 0;
        std::vector<uint8_t> png;
        if ((err = lodepng::encode(png, tiles, chunkDims.x, chunkDims.y, state)) ||
            (err = lodepng::save_file(png, fullPath))) {
            // Encoding or saving failed
            throw std::runtime_error{lodepng_error_text(err)};
        }
    }
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
