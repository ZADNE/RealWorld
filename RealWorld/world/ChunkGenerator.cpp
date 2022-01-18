#include <RealWorld/world/ChunkGenerator.hpp>

#include <vector>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/Vertex.hpp>
#include <RealEngine/ResourceManager.hpp>
#include <RealEngine/UniformManager.hpp>

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
	//Inits
	initObjects();
	initShaders();
}

ChunkGenerator::~ChunkGenerator() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
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
}

Chunk ChunkGenerator::generateChunk(glm::ivec2 posCh, GLuint uploadTexture, glm::ivec2 offset) {
	m_randomGen.seed(m_seed);
	ChunkUniforms chunkUniforms;
	chunkUniforms.chunkOffsetBc = static_cast<glm::vec2>(posCh) * m_chunkDims_f;
	RE::UniformManager::std.setUniformBuffer(
		"ChunkUniforms", offsetof(ChunkUniforms, chunkOffsetBc), sizeof(chunkUniforms.chunkOffsetBc), &chunkUniforms.chunkOffsetBc);


	m_genSurf[0].setTarget();
	glDisable(GL_POINT_SPRITE);
	glBindVertexArray(m_VAO);
	setVBOToWholeChunk();

	//Actual generation
	generateBasicTerrain();
	cellularAutomaton();
	setVars();


	glEnable(GL_POINT_SPRITE);
	glBindVertexArray(0);

	std::vector<unsigned char> data;
	data.resize((size_t)m_chunkDims.x * m_chunkDims.y * 4ull);
	//Download data -> CPU stall -> Pixel Buffer Object todo
	glReadPixels(BORDER_WIDTH, BORDER_WIDTH, m_chunkDims.x, m_chunkDims.y, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, data.data());
	glCopyTextureSubImage2D(uploadTexture, 0, offset.x, offset.y, BORDER_WIDTH, BORDER_WIDTH, m_chunkDims.x, m_chunkDims.y);

	m_genSurf[0].resetTarget();

	return Chunk(posCh, m_chunkDims, std::move(data));
}

void ChunkGenerator::initShaders() {
	RE::UniformManager::std.addUniformBuffer("ChunkUniforms", sizeof(ChunkUniforms));
	RE::UniformManager::std.addShader("ChunkUniforms", m_varShader.get());
	RE::UniformManager::std.addShader("ChunkUniforms", m_setShader.get());
	RE::UniformManager::std.addShader("ChunkUniforms", m_basicTerrainShader.get());
	RE::UniformManager::std.addShader("ChunkUniforms", m_cellularAutomatonShader.get());

	//SET shader

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

void ChunkGenerator::initObjects() {
	//Generating OpenGL objects
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	//Setting up VAO
	glBindVertexArray(m_VAO);
	glEnableVertexAttribArray(RE::ATTLOC_PO);
	//Setting up VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	//Position
	glVertexAttribPointer(RE::ATTLOC_PO, 2, GL_FLOAT, GL_FALSE, sizeof(RE::VertexPO), (void*)offsetof(RE::VertexPO, position));

	glBindVertexArray(0);
}

void ChunkGenerator::setVBOToWholeChunk() {
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	RE::VertexPO vertices[6];
	//Left-top triangle
	vertices[0].setPosition(-BORDER_WIDTH, -BORDER_WIDTH);
	vertices[1].setPosition(m_chunkDims_f.x + BORDER_WIDTH, m_chunkDims_f.y + BORDER_WIDTH);
	vertices[2].setPosition(-BORDER_WIDTH, m_chunkDims_f.y + BORDER_WIDTH);
	//Right-bottom triangle
	vertices[3].setPosition(-BORDER_WIDTH, -BORDER_WIDTH);
	vertices[4].setPosition(m_chunkDims_f.x + BORDER_WIDTH, m_chunkDims_f.y + BORDER_WIDTH);
	vertices[5].setPosition(m_chunkDims_f.x + BORDER_WIDTH, -BORDER_WIDTH);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0]), GL_STREAM_DRAW);
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
	m_basicTerrainShader->setShader();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	m_basicTerrainShader->resetShader();
}

void ChunkGenerator::cellularAutomaton() {
	GLuint surfIndex = 1u;
	auto pass = [this, &surfIndex](GLuint low, GLuint high, size_t passes) {
		glUniform1ui(WGS::LOC_CELL_AUTO_LOW, low);
		glUniform1ui(WGS::LOC_CELL_AUTO_HIGH, high);
		for (size_t i = 0; i < passes; i++) {
			m_genSurf[surfIndex % m_genSurf.size()].setTarget();
			glUniform1ui(WGS::LOC_TILES_SELECTOR, surfIndex);
			surfIndex++;
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	};
	auto doublePass = [this, &surfIndex, pass](GLuint firstlow, GLuint firsthigh, GLuint secondlow, GLuint secondhigh, size_t passes) {
		for (size_t i = 0; i < passes; i++) {
			pass(firstlow, firsthigh, 1);
			pass(secondlow, secondhigh, 1);
		}
	};

	m_cellularAutomatonShader->setShader();

	RE::SurfaceTargetTextures stt{};
	stt.targetTexture(0);
	m_genSurf[0].setTargetTextures(stt);

	doublePass(3, 4, 4, 5, 4);


	m_cellularAutomatonShader->resetShader();

	stt.targetTexture(1);
	m_genSurf[0].setTarget();
	m_genSurf[0].setTargetTextures(stt);
	glTextureBarrier();
}

void ChunkGenerator::setVars() {
	m_varShader->setShader();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	m_varShader->resetShader();
}
