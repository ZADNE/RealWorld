#include <RealWorld/world/ChunkGenerator.hpp>

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/UniformManager.hpp>

#include <RealWorld/world//TDB.hpp>
#include <RealWorld/items/Item.hpp>
#include <RealWorld/div.hpp>

const int BORDER_WIDTH = 16;

struct ChunkUniforms {
	glm::mat4 chunkGenMatrix;
	glm::vec2 chunkOffsetBc;
	glm::vec2 chunkDims;
	glm::vec2 chunkBorders;
	int seed;
};

ChunkGenerator::ChunkGenerator() {
	initShaders();
	initVAO();
}

ChunkGenerator::~ChunkGenerator() {

}

void ChunkGenerator::setTargetWorld(int seed, glm::uvec2 chunkDims, glm::uvec2 activeChunksRect) {
	m_seed = seed;
	m_chunkDims = chunkDims;
	m_chunkDims_f = static_cast<glm::vec2>(chunkDims);
	m_activeChunksRect = static_cast<glm::ivec2>(activeChunksRect);
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
	m_randomGen.seed(m_seed);
	ChunkUniforms chunkUniforms;
	chunkUniforms.chunkOffsetBc = static_cast<glm::vec2>(posCh) * m_chunkDims_f;
	RE::UniformManager::std.setUniformBuffer(
		"ChunkUniforms", offsetof(ChunkUniforms, chunkOffsetBc), sizeof(chunkUniforms.chunkOffsetBc), &chunkUniforms.chunkOffsetBc);


	m_genSurf[0].setTarget();

	m_VAO.bind();

	//Actual generation
	generateBasicTerrain();
	cellularAutomaton();
	setVars();


	m_VAO.unbind();

	std::vector<unsigned char> data;
	data.resize((size_t)m_chunkDims.x * m_chunkDims.y * 4ull);
	//Download data -> CPU stall -> Pixel Buffer Object todo
	glReadPixels(BORDER_WIDTH, BORDER_WIDTH, m_chunkDims.x, m_chunkDims.y, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, data.data());

	glCopyTextureSubImage2D(uploadTexture, 0, offset.x, offset.y, BORDER_WIDTH, BORDER_WIDTH, m_chunkDims.x, m_chunkDims.y);

	m_genSurf[0].resetTarget();

	return Chunk(posCh, m_chunkDims, data);
}

void ChunkGenerator::initShaders() {
	RE::UniformManager::std.addUniformBuffer("ChunkUniforms", sizeof(ChunkUniforms));
	RE::UniformManager::std.addShader("ChunkUniforms", m_varShader.get());
	RE::UniformManager::std.addShader("ChunkUniforms", m_basicTerrainShader.get());
	RE::UniformManager::std.addShader("ChunkUniforms", m_cellularAutomatonShader.get());

	//BASIC TERRAIN shader

	//VAR shader
	m_varShader->setUniform("air", glm::uvec2(BLOCK_ID::AIR, WALL_ID::AIR));
	m_varShader->setUniform(WGS::LOC_WORLD_TEXTURE, TEX_UNIT_CHUNK_TILES1.index());

	//CELLULAR AUTOMATON shader
	m_cellularAutomatonShader->setUniform(WGS::LOC_TILES0_TEXTURE, TEX_UNIT_CHUNK_TILES0.index());
	m_cellularAutomatonShader->setUniform(WGS::LOC_TILES1_TEXTURE, TEX_UNIT_CHUNK_TILES1.index());
	m_cellularAutomatonShader->setUniform(WGS::LOC_MATERIAL_TEXTURE, TEX_UNIT_CHUNK_MATERIAL.index());
	m_cellularAutomatonShader->setUniform(shaders::LOC_AIR_ID, glm::uvec4((GLuint)BLOCK_ID::AIR, 0, (GLuint)WALL_ID::AIR, 0));
}

void ChunkGenerator::initVAO() {
	m_VAO.setBindingPoint(0u, m_VBO, 0, sizeof(RE::VertexPO));

	m_VAO.setAttribute(RE::ATTR_POSITION, RE::VertexComponentCount::XY, RE::VertexComponentType::FLOAT, offsetof(RE::VertexPO, position));

	m_VAO.connectAttributeToBindingPoint(RE::ATTR_POSITION, 0u);
}

void ChunkGenerator::setVBOToWholeChunk() {
	const RE::VertexPO vertices[6] = {
		//Left-top triangle
		glm::vec2{-BORDER_WIDTH, -BORDER_WIDTH},
		glm::vec2{m_chunkDims_f.x + BORDER_WIDTH, m_chunkDims_f.y + BORDER_WIDTH},
		glm::vec2{-BORDER_WIDTH, m_chunkDims_f.y + BORDER_WIDTH},
		//Right-bottom triangle
		glm::vec2{-BORDER_WIDTH, -BORDER_WIDTH},
		glm::vec2{m_chunkDims_f.x + BORDER_WIDTH, m_chunkDims_f.y + BORDER_WIDTH},
		glm::vec2{m_chunkDims_f.x + BORDER_WIDTH, -BORDER_WIDTH}
	};
	m_VBO.overwrite(sizeof(vertices), vertices);
}

void ChunkGenerator::updateUniformsAfterSetTarget() {
	ChunkUniforms uni;
	uni.chunkGenMatrix = glm::ortho(static_cast<float>(-BORDER_WIDTH), m_chunkDims_f.x + BORDER_WIDTH, static_cast<float>(-BORDER_WIDTH), m_chunkDims_f.y + BORDER_WIDTH);
	uni.chunkDims = m_chunkDims_f;
	uni.chunkBorders = glm::vec2(BORDER_WIDTH, BORDER_WIDTH);
	uni.seed = m_seed;
	RE::UniformManager::std.setUniformBuffer("ChunkUniforms", 0u, sizeof(uni), &uni);
}

void ChunkGenerator::generateBasicTerrain() {
	m_basicTerrainShader->use();
	m_VAO.renderArrays(RE::Primitive::TRIANGLES, 0, 6);
	m_basicTerrainShader->unuse();
}

void ChunkGenerator::cellularAutomaton() {
	GLuint surfIndex = 1u;
	auto pass = [this, &surfIndex](GLuint low, GLuint high, size_t passes) {
		m_cellularAutomatonShader->setUniform(WGS::LOC_CELL_AUTO_LOW, low);
		m_cellularAutomatonShader->setUniform(WGS::LOC_CELL_AUTO_HIGH, high);
		for (size_t i = 0; i < passes; i++) {
			m_genSurf[surfIndex % m_genSurf.size()].setTarget();
			m_cellularAutomatonShader->setUniform(WGS::LOC_TILES_SELECTOR, surfIndex);
			surfIndex++;
			m_VAO.renderArrays(RE::Primitive::TRIANGLES, 0, 6);
		}
	};
	auto doublePass = [this, &surfIndex, pass](GLuint firstlow, GLuint firsthigh, GLuint secondlow, GLuint secondhigh, size_t passes) {
		for (size_t i = 0; i < passes; i++) {
			pass(firstlow, firsthigh, 1);
			pass(secondlow, secondhigh, 1);
		}
	};

	m_cellularAutomatonShader->use();

	RE::SurfaceTargetTextures stt{};
	stt.targetTexture(0);
	m_genSurf[0].setTargetTextures(stt);

	doublePass(3, 4, 4, 5, 4);

	m_cellularAutomatonShader->unuse();

	stt.targetTexture(1);
	m_genSurf[0].setTarget();
	m_genSurf[0].setTargetTextures(stt);
}

void ChunkGenerator::setVars() {
	m_varShader->use();
	m_VAO.renderArrays(RE::Primitive::TRIANGLES, 0, 6);
	m_varShader->unuse();
}
