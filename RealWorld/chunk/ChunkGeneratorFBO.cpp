/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/chunk/ChunkGeneratorFBO.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

ChunkGeneratorFBO::ChunkGeneratorFBO() {
	p_chunkUniformBuffer.connectToInterfaceBlock(m_structureShader, 0u);
	p_chunkUniformBuffer.connectToInterfaceBlock(m_consolidationShader, 0u);
	p_chunkUniformBuffer.connectToInterfaceBlock(m_variantSelectionShader, 0u);

	m_genSurf[0].getTexture(0).bind(TEX_UNIT_GEN_TILES[0]);
	m_genSurf[1].getTexture(0).bind(TEX_UNIT_GEN_TILES[1]);
	m_genSurf[0].getTexture(1).bind(TEX_UNIT_GEN_MATERIAL);
}

ChunkGeneratorFBO::~ChunkGeneratorFBO() {

}

void ChunkGeneratorFBO::prepareToGenerate() {
	m_VAO.bind();
	m_genSurf[0].setTarget();
}

void ChunkGeneratorFBO::generateBasicTerrain() {
	m_structureShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_structureShader.unuse();
}

void ChunkGeneratorFBO::consolidateEdges() {
	GLuint cycleN = 0u;
	auto pass = [this, &cycleN](const glm::ivec2& thresholds, size_t passes) {
		m_consolidationShader.setUniform(LOC_THRESHOLDS, thresholds);
		for (size_t i = 0; i < passes; i++) {
			m_consolidationShader.setUniform(LOC_CYCLE_N, cycleN++);
			m_genSurf[(cycleN + 1) % m_genSurf.size()].setTarget();
			glTextureBarrier();
			m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
		}
	};
	auto doublePass = [pass](const glm::ivec2& firstThresholds, const glm::ivec2& secondThresholds, size_t passes) {
		for (size_t i = 0; i < passes; i++) {
			pass(firstThresholds, 1);
			pass(secondThresholds, 1);
		}
	};

	RE::SurfaceTargetTextures stt{};
	stt.targetTexture(0, 0);
	m_genSurf[0].setTargetTextures(stt);

	m_consolidationShader.use();
	doublePass({3, 4}, {4, 5}, 4);
	m_consolidationShader.unuse();

	stt.targetTexture(1, 1);
	m_genSurf[0].setTarget();
	m_genSurf[0].setTargetTextures(stt);
	assert(static_cast<int>(cycleN) <= BORDER_WIDTH);
}

void ChunkGeneratorFBO::selectVariants() {
	glTextureBarrier();
	m_variantSelectionShader.use();
	m_VAO.renderArrays(TRIANGLE_STRIP, 0, 4);
	m_variantSelectionShader.unuse();
}

void ChunkGeneratorFBO::finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) {
	m_genSurf[0].resetTarget();
	m_VAO.unbind();
	m_genSurf[0].getTexture().copyTexelsBetweenImages(0, glm::ivec2{BORDER_WIDTH}, destinationTexture, 0, destinationOffset, iCHUNK_SIZE);
}
