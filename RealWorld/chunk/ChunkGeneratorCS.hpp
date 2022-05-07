/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>

#include <RealEngine/graphics/textures/Texture.hpp>
#include <RealEngine/graphics/ShaderProgram.hpp>

#include <RealWorld/chunk/ChunkGenerator.hpp>
#include <RealWorld/shaders/generation.hpp>
#include <RealWorld/constants/chunk.hpp>

/**
 * @brief Generates new chunks by compute shaders.
 */
class ChunkGeneratorCS: public ChunkGenerator {
public:

	ChunkGeneratorCS();
	~ChunkGeneratorCS();

private:

	void prepareToGenerate() override;
	void generateBasicTerrain() override;
	void consolidateEdges() override;
	void selectVariants() override;
	void finishGeneration(const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset) override;


	RE::ShaderProgram m_structureShader = RE::ShaderProgram{{.comp = structure_comp}};
	RE::ShaderProgram m_consolidationShader = RE::ShaderProgram{{.comp = consolidation_comp}};
	RE::ShaderProgram m_variantSelectionShader = RE::ShaderProgram{{.comp = variantSelection_comp}};

	std::array<RE::Texture, 2> m_tilesTex = {
		RE::Texture{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}},
		RE::Texture{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}}
	};
	RE::Texture m_materialGenTex{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}};
};