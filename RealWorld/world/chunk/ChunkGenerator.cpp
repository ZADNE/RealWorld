#include <RealWorld/world/chunk/ChunkGenerator.hpp>

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world//TDB.hpp>
#include <RealWorld/items/Item.hpp>
#include <RealWorld/div.hpp>
#include <RealWorld/rendering/ImageUnits.hpp>


ChunkGenerator::ChunkGenerator() {
	m_chunkUniformBuffer.connectToShaderProgram(m_basicTerrainShader, 0u);
	//m_chunkUniformBuffer.connectToShaderProgram(m_cellularAutomatonShader, 0u);
	m_chunkUniformBuffer.connectToShaderProgram(m_selectVariationShader, 0u);

	m_tilesGenSurf.getTexture().bindImage(IMG_UNIT_GEN_TILES, 0, RE::ImageAccess::READ_WRITE);
	m_materialGenTex.bindImage(IMG_UNIT_GEN_MATERIAL, 0, RE::ImageAccess::READ_WRITE);
}

ChunkGenerator::~ChunkGenerator() {

}

void ChunkGenerator::setSeed(int seed) {
	m_seed = seed;
	m_chunkUniformBuffer.overwrite(offsetof(ChunkUniforms, seed), sizeof(seed), &seed);
}

Chunk ChunkGenerator::generateChunk(const glm::ivec2& posCh, const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) {
	//Update chunk offset within the uniform buffer
	glm::ivec2 offsetTi = posCh * CHUNK_SIZE;
	m_chunkUniformBuffer.overwrite(offsetof(ChunkUniforms, chunkOffsetTi), sizeof(offsetTi), &offsetTi);

	//Actual generation
	generateBasicTerrain();
	cellularAutomaton();
	selectVariations();

	std::vector<unsigned char> data;
	data.resize(static_cast<size_t>(CHUNK_SIZE.x) * CHUNK_SIZE.y * 4ull);
	m_tilesGenSurf.setTarget();
	//Download data -> CPU stall -> Pixel Buffer Object todo
	glReadnPixels(BORDER_WIDTH, BORDER_WIDTH, CHUNK_SIZE.x, CHUNK_SIZE.y, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, static_cast<GLsizei>(data.size()), data.data());
	m_tilesGenSurf.resetTarget();

	m_tilesGenSurf.getTexture().copyTexelsBetweenImages(0, glm::ivec2{BORDER_WIDTH}, destinationTexture, 0, destinationOffset, CHUNK_SIZE);

	return Chunk(posCh, data);
}

void ChunkGenerator::generateBasicTerrain() {
	m_basicTerrainShader.dispatchCompute({GEN_CHUNK_SIZE, 1}, true);
}

void ChunkGenerator::cellularAutomaton() {
	m_cellularAutomatonShader.dispatchCompute({1, 1, 1}, true);
}

void ChunkGenerator::selectVariations() {
	m_selectVariationShader.dispatchCompute({GEN_CHUNK_SIZE, 1}, true);
}
