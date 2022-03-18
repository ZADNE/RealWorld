﻿#include <RealWorld/world/chunk/ChunkGenerator.hpp>

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world//TDB.hpp>
#include <RealWorld/items/Item.hpp>
#include <RealWorld/div.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>
#include <RealWorld/rendering/ImageUnits.hpp>

const int GEN_CS_GROUP_SIZE = 16;

ChunkGenerator::ChunkGenerator() {
	m_chunkUniformBuffer.connectToShaderProgram(m_basicTerrainShader, 0u);
	m_chunkUniformBuffer.connectToShaderProgram(m_selectVariationShader, 0u);

#ifdef GEN_USE_COMP
	m_tilesTex[0].bind(TEX_UNIT_GEN_TILES[0]);
	m_tilesTex[1].bind(TEX_UNIT_GEN_TILES[1]);
	m_materialGenTex.bind(TEX_UNIT_GEN_MATERIAL);

	m_tilesTex[0].bindImage(IMG_UNIT_GEN_TILES[0], 0, RE::ImageAccess::READ_WRITE);
	m_tilesTex[1].bindImage(IMG_UNIT_GEN_TILES[1], 0, RE::ImageAccess::READ_WRITE);
	m_materialGenTex.bindImage(IMG_UNIT_GEN_MATERIAL, 0, RE::ImageAccess::READ_WRITE);
#else
	m_chunkUniformBuffer.connectToShaderProgram(m_cellularAutomatonShader, 0u);

	m_genSurf[0].getTexture(0).bind(TEX_UNIT_GEN_TILES[0]);
	m_genSurf[1].getTexture(0).bind(TEX_UNIT_GEN_TILES[1]);
	m_genSurf[0].getTexture(1).bind(TEX_UNIT_GEN_MATERIAL);
#endif
}

ChunkGenerator::~ChunkGenerator() {

}

void ChunkGenerator::setSeed(int seed) {
	m_seed = seed;
	m_chunkUniformBuffer.overwrite(offsetof(ChunkUniforms, seed), sizeof(seed), &seed);
}

void ChunkGenerator::generateChunk(const glm::ivec2& posCh, const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) {
	//Update chunk offset within the uniform buffer
	glm::ivec2 offsetTi = posCh * CHUNK_SIZE;
	m_chunkUniformBuffer.overwrite(offsetof(ChunkUniforms, chunkOffsetTi), sizeof(offsetTi), &offsetTi);

#ifndef GEN_USE_COMP
	m_VAO.bind();
	m_genSurf[0].setTarget();
#endif

	//Actual generation
	generateBasicTerrain();
	cellularAutomaton();
	selectVariations();


#ifdef GEN_USE_COMP
	m_tilesTex[0].copyTexelsBetweenImages(0, glm::ivec2{BORDER_WIDTH}, destinationTexture, 0, destinationOffset, CHUNK_SIZE);
#else
	m_genSurf[0].resetTarget();
	m_VAO.unbind();
	m_genSurf[0].getTexture().copyTexelsBetweenImages(0, glm::ivec2{BORDER_WIDTH}, destinationTexture, 0, destinationOffset, CHUNK_SIZE);
#endif
}

void ChunkGenerator::generateBasicTerrain() {
#ifdef GEN_USE_COMP
	m_basicTerrainShader.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
#else
	m_basicTerrainShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_basicTerrainShader.unuse();
#endif
}

void ChunkGenerator::cellularAutomaton() {
	GLuint cycleN = 0u;
	auto pass = [this, &cycleN](const glm::ivec2& thresholds, size_t passes) {
		m_cellularAutomatonShader.setUniform(LOC_THRESHOLDS, thresholds);
		for (size_t i = 0; i < passes; i++) {
			m_cellularAutomatonShader.setUniform(LOC_CYCLE_N, cycleN++);
		#ifdef GEN_USE_COMP
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			m_cellularAutomatonShader.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, false);
		#else
			m_genSurf[(cycleN + 1) % m_genSurf.size()].setTarget();
			glTextureBarrier();
			m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
		#endif
		}
	};
	auto doublePass = [pass](const glm::ivec2& firstThresholds, const glm::ivec2& secondThresholds, size_t passes) {
		for (size_t i = 0; i < passes; i++) {
			pass(firstThresholds, 1);
			pass(secondThresholds, 1);
		}
	};

#ifndef GEN_USE_COMP
	RE::SurfaceTargetTextures stt{};
	stt.targetTexture(0);
	m_genSurf[0].setTargetTextures(stt);
#endif

	m_cellularAutomatonShader.use();
	doublePass({3, 4}, {4, 5}, 4);
	m_cellularAutomatonShader.unuse();

#ifndef GEN_USE_COMP
	stt.targetTexture(1);
	m_genSurf[0].setTarget();
	m_genSurf[0].setTargetTextures(stt);
#endif
	assert(static_cast<int>(cycleN) <= BORDER_WIDTH);
}

void ChunkGenerator::selectVariations() {
#ifdef GEN_USE_COMP
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	m_selectVariationShader.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
#else
	m_selectVariationShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_selectVariationShader.unuse();
#endif
}