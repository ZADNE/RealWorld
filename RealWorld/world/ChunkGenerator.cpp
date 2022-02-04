#include <RealWorld/world/ChunkGenerator.hpp>

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world//TDB.hpp>
#include <RealWorld/items/Item.hpp>
#include <RealWorld/div.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>

const int BORDER_WIDTH = 8;

ChunkGenerator::ChunkGenerator() {
	initShaders();
	initVAO();
}

ChunkGenerator::~ChunkGenerator() {

}

void ChunkGenerator::setTargetWorld(int seed, glm::uvec2 chunkDims) {
	m_seed = seed;
	m_chunkDimsTi = static_cast<glm::vec2>(chunkDims);
	m_genSurf[0].resize({chunkDims + glm::uvec2(BORDER_WIDTH, BORDER_WIDTH) * 2u}, 2u);
	m_genSurf[1].resize({chunkDims + glm::uvec2(BORDER_WIDTH, BORDER_WIDTH) * 2u}, 1u);
	updateUniformsAfterSetTarget();

	//Bind new textures to texture units
	TEX_UNIT_CHUNK_TILES0.setActive();
	m_genSurf[1].bindTexture(0);
	TEX_UNIT_CHUNK_TILES1.setActive();
	m_genSurf[0].bindTexture(0);
	TEX_UNIT_CHUNK_MATERIAL.setActive();
	m_genSurf[0].bindTexture(1);
	TEX_UNIT_VOLATILE.setActive();

	//Update VBO to new chunk size
	setVBOToWholeChunk();
}

Chunk ChunkGenerator::generateChunk(glm::ivec2 posCh, GLuint uploadTexture, glm::ivec2 offset) {
	//Update chunk offset with uniform buffer
	glm::vec2 chunkOffsetTi = static_cast<glm::vec2>(posCh) * m_chunkDimsTi;
	m_chunkUniformBuffer.overwrite(offsetof(ChunkUniforms, chunkOffsetTi), sizeof(chunkOffsetTi), &chunkOffsetTi);

	m_genSurf[0].setTarget();

	m_VAO.bind();

	//Actual generation
	generateBasicTerrain();
	cellularAutomaton();
	selectVariations();


	m_VAO.unbind();

	std::vector<unsigned char> data;
	glm::uvec2 chunkDimsTi = m_chunkDimsTi;
	data.resize((size_t)chunkDimsTi.x * chunkDimsTi.y * 4ull);
	//Download data -> CPU stall -> Pixel Buffer Object todo
	glReadPixels(BORDER_WIDTH, BORDER_WIDTH, chunkDimsTi.x, chunkDimsTi.y, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, data.data());

	glCopyTextureSubImage2D(uploadTexture, 0, offset.x, offset.y, BORDER_WIDTH, BORDER_WIDTH, chunkDimsTi.x, chunkDimsTi.y);

	m_genSurf[0].resetTarget();

	return Chunk(posCh, chunkDimsTi, data);
}

void ChunkGenerator::initShaders() {
	m_chunkUniformBuffer.connectToShaderProgram(m_basicTerrainShader, 0u);
	m_chunkUniformBuffer.connectToShaderProgram(m_cellularAutomatonShader, 0u);
	m_chunkUniformBuffer.connectToShaderProgram(m_selectVariationShader, 0u);

	m_selectVariationShader.setUniform(LOC_WORLD_TEXTURE, TEX_UNIT_CHUNK_TILES1);

	m_cellularAutomatonShader.setUniform(LOC_TILES0_TEXTURE, TEX_UNIT_CHUNK_TILES0);
	m_cellularAutomatonShader.setUniform(LOC_TILES1_TEXTURE, TEX_UNIT_CHUNK_TILES1);
	m_cellularAutomatonShader.setUniform(LOC_MATERIAL_TEXTURE, TEX_UNIT_CHUNK_MATERIAL);
}

void ChunkGenerator::initVAO() {
	m_VAO.setBindingPoint(0u, m_VBO, 0, sizeof(RE::VertexPO));

	m_VAO.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(RE::VertexPO, position));

	m_VAO.connectAttributeToBindingPoint(RE::ATTR_POSITION, 0u);
}

void ChunkGenerator::setVBOToWholeChunk() {
	const RE::VertexPO vertices[4] = {
		glm::vec2{-BORDER_WIDTH, -BORDER_WIDTH},
		glm::vec2{m_chunkDimsTi.x + BORDER_WIDTH, -BORDER_WIDTH},
		glm::vec2{-BORDER_WIDTH, m_chunkDimsTi.y + BORDER_WIDTH},
		glm::vec2{m_chunkDimsTi.x + BORDER_WIDTH, m_chunkDimsTi.y + BORDER_WIDTH}
	};
	m_VBO.overwrite(0, sizeof(vertices), vertices);
}

void ChunkGenerator::updateUniformsAfterSetTarget() {
	ChunkUniforms uni{
		.chunkGenMatrix = glm::ortho(static_cast<float>(-BORDER_WIDTH), m_chunkDimsTi.x + BORDER_WIDTH, static_cast<float>(-BORDER_WIDTH), m_chunkDimsTi.y + BORDER_WIDTH),
		.chunkDimsTi = m_chunkDimsTi,
		.chunkBorders = glm::vec2(BORDER_WIDTH, BORDER_WIDTH),
		.seed = static_cast<float>(m_seed)
	};
	m_chunkUniformBuffer.overwrite(uni);
}

void ChunkGenerator::generateBasicTerrain() {
	m_basicTerrainShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_basicTerrainShader.unuse();
}

void ChunkGenerator::cellularAutomaton() {
	GLuint surfIndex = 1u;
	auto pass = [this, &surfIndex](GLuint low, GLuint high, size_t passes) {
		m_cellularAutomatonShader.setUniform(LOC_CELL_AUTO_LOW, low);
		m_cellularAutomatonShader.setUniform(LOC_CELL_AUTO_HIGH, high);
		for (size_t i = 0; i < passes; i++) {
			m_genSurf[surfIndex % m_genSurf.size()].setTarget();
			m_cellularAutomatonShader.setUniform(LOC_TILES_SELECTOR, surfIndex);
			surfIndex++;
			m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
		}
	};
	auto doublePass = [this, &surfIndex, pass](GLuint firstlow, GLuint firsthigh, GLuint secondlow, GLuint secondhigh, size_t passes) {
		for (size_t i = 0; i < passes; i++) {
			pass(firstlow, firsthigh, 1);
			pass(secondlow, secondhigh, 1);
		}
	};

	m_cellularAutomatonShader.use();

	RE::SurfaceTargetTextures stt{};
	stt.targetTexture(0);
	m_genSurf[0].setTargetTextures(stt);

	doublePass(3, 4, 4, 5, 4);

	m_cellularAutomatonShader.unuse();

	stt.targetTexture(1);
	m_genSurf[0].setTarget();
	m_genSurf[0].setTargetTextures(stt);
}

void ChunkGenerator::selectVariations() {
	m_selectVariationShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_selectVariationShader.unuse();
}
