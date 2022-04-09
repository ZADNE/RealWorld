#include <RealWorld/save/ChunkLoader.hpp>

#include <stdexcept>

#include <RealEngine/external/lodepng/lodepng.hpp>

std::vector<unsigned char> ChunkLoader::loadChunk(const std::string& folderPath, glm::ivec2 chunkPos, glm::uvec2 chunkDims) {
	std::vector<unsigned char> bytes;

	glm::uvec2 realDims;

	std::string fullPath = folderPath + chunkToChunkFilename(chunkPos);

	unsigned int error = lodepng::decode(bytes, realDims.x, realDims.y, fullPath, LodePNGColorType::LCT_RGBA, 8u);
	if (error) throw std::runtime_error("Error loading chunk file " + fullPath);
	if (chunkDims != realDims) throw std::runtime_error("Unexpected size of chunk");

	return bytes;
}

void ChunkLoader::saveChunk(const std::string& folderPath, glm::ivec2 chunkPos, glm::uvec2 chunkDims, const std::vector<unsigned char>& data) {
	std::string fullPath = folderPath + chunkToChunkFilename(chunkPos);

#ifndef _DEBUG
	unsigned int error = lodepng::encode(fullPath, data, chunkDims.x, chunkDims.y, LodePNGColorType::LCT_RGBA, 8u);

	if (error) throw std::runtime_error("Error encoding or saving chunk " + fullPath);
#endif // ! _DEBUG

}

std::string ChunkLoader::chunkToChunkFilename(glm::ivec2 chunkPos) {
	return "chunk_" + std::to_string(chunkPos.x) + "x" + std::to_string(chunkPos.y) + ".chunk";
}
