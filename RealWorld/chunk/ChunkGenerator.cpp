/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/chunk/ChunkGenerator.hpp>

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>
#include <RealWorld/reserved_units/TextureUnits.hpp>
#include <RealWorld/reserved_units/ImageUnits.hpp>

const int GEN_CS_GROUP_SIZE = 16;

ChunkGenerator::ChunkGenerator() {
	m_chunkUniformBuffer.connectToShaderProgram(m_structureShader, 0u);
	m_chunkUniformBuffer.connectToShaderProgram(m_variantSelectionShader, 0u);

#ifndef GEN_USE_FRAMEBUFFER
	m_tilesTex[0].bind(TEX_UNIT_GEN_TILES[0]);
	m_tilesTex[1].bind(TEX_UNIT_GEN_TILES[1]);
	m_materialGenTex.bind(TEX_UNIT_GEN_MATERIAL);

	m_tilesTex[0].bindImage(IMG_UNIT_GEN_TILES[0], 0, RE::ImageAccess::READ_WRITE);
	m_tilesTex[1].bindImage(IMG_UNIT_GEN_TILES[1], 0, RE::ImageAccess::READ_WRITE);
	m_materialGenTex.bindImage(IMG_UNIT_GEN_MATERIAL, 0, RE::ImageAccess::READ_WRITE);
#else
	m_chunkUniformBuffer.connectToShaderProgram(m_consolidationShader, 0u);

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
	glm::ivec2 offsetTi = posCh * iCHUNK_SIZE;
	m_chunkUniformBuffer.overwrite(offsetof(ChunkUniforms, chunkOffsetTi), sizeof(offsetTi), &offsetTi);

#ifdef GEN_USE_FRAMEBUFFER
	m_VAO.bind();
	m_genSurf[0].setTarget();
#endif

	//Actual generation
	generateBasicTerrain();
	consolidateEdges();
	selectVariants();


#ifndef GEN_USE_FRAMEBUFFER
	m_tilesTex[0].copyTexelsBetweenImages(0, glm::ivec2{BORDER_WIDTH}, destinationTexture, 0, destinationOffset, iCHUNK_SIZE);
#else
	m_genSurf[0].resetTarget();
	m_VAO.unbind();
	m_genSurf[0].getTexture().copyTexelsBetweenImages(0, glm::ivec2{BORDER_WIDTH}, destinationTexture, 0, destinationOffset, iCHUNK_SIZE);
#endif
}

void ChunkGenerator::generateBasicTerrain() {
#ifndef GEN_USE_FRAMEBUFFER
	m_structureShader.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
#else
	m_structureShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_structureShader.unuse();
#endif
}

void ChunkGenerator::consolidateEdges() {
	GLuint cycleN = 0u;
	auto pass = [this, &cycleN](const glm::ivec2& thresholds, size_t passes) {
		m_consolidationShader.setUniform(LOC_THRESHOLDS, thresholds);
		for (size_t i = 0; i < passes; i++) {
			m_consolidationShader.setUniform(LOC_CYCLE_N, cycleN++);
		#ifndef GEN_USE_FRAMEBUFFER
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			m_consolidationShader.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, false);
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

#ifdef GEN_USE_FRAMEBUFFER
	RE::SurfaceTargetTextures stt{};
	stt.targetTexture(0, 0);
	m_genSurf[0].setTargetTextures(stt);
#endif

	m_consolidationShader.use();
	doublePass({3, 4}, {4, 5}, 4);
	m_consolidationShader.unuse();

#ifdef GEN_USE_FRAMEBUFFER
	stt.targetTexture(1, 1);
	m_genSurf[0].setTarget();
	m_genSurf[0].setTargetTextures(stt);
#endif
	assert(static_cast<int>(cycleN) <= BORDER_WIDTH);
}

void ChunkGenerator::selectVariants() {
#ifndef GEN_USE_FRAMEBUFFER
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	m_variantSelectionShader.dispatchCompute({GEN_CHUNK_SIZE / GEN_CS_GROUP_SIZE, 1}, true);
#else
	glTextureBarrier();
	m_variantSelectionShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_variantSelectionShader.unuse();
#endif
}
