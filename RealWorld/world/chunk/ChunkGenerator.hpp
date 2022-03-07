#pragma once
#include <array>

#include <RealEngine/graphics/ShaderProgram.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/shaders/chunk_generation.hpp>
#include <RealWorld/rendering/UniformBuffers.hpp>
#include <RealWorld/constants/chunk.hpp>

#define GEN_USE_COMP

/**
 * Generates new chunks.
 */
class ChunkGenerator {
public:
	/**
	 * @brief Width of the padding around the generated chunk.
	 *
	 * Avoids errors at the edges of generated chunks.
	*/
	const int BORDER_WIDTH = 8;

	/**
	 * @brief Size of the area that is generated for each chunk
	 *
	 * It is bigger than the actual chunk because it contains the padding around.
	 *
	 * @see BORDER_WIDTH
	*/
	const glm::ivec2 GEN_CHUNK_SIZE = CHUNK_SIZE + 2 * BORDER_WIDTH;

	/**
	 * @brief Contructs new chunk generator, initializes world-independent members.
	 */
	ChunkGenerator();

	/**
	 * @brief Decontructs chunk generator, frees all memory.
	 */
	~ChunkGenerator();

	/**
	 * @brief Sets the seed that controls how the generated chunks look
	 *
	 * @param seed Seed of the world
	 */
	void setSeed(int seed);

	/**
	 * @brief Generates a chunk. The pixels are stored to given texture at given position.
	 *
	 * @param posCh Position of the chunk (measured in chunks)
	 * @param destinationTexture The texture that will receive the generated chunk
	 * @param destinationOffset Offset within destinationTexture where the texels/tiles will be copied
	 */
	void generateChunk(const glm::ivec2& posCh, const RE::Texture& destinationTexture, const glm::ivec2& destinationOffset);

private:
	using enum RE::BufferType;
	using enum RE::BufferStorage;
	using enum RE::BufferAccessFrequency;
	using enum RE::BufferAccessNature;
	using enum RE::BufferUsageFlags;
	using enum RE::VertexComponentCount;
	using enum RE::VertexComponentType;
	using enum RE::Primitive;
	struct ChunkUniforms {
		glm::ivec2 chunkOffsetTi;
		int seed;
	};
	RE::UniformBuffer m_chunkUniformBuffer{UNIF_BUF_CHUNKGEN, true, RE::BufferUsageFlags::DYNAMIC_STORAGE, sizeof(ChunkUniforms)};

	//Actual generation functions
	void generateBasicTerrain();
	void cellularAutomaton();
	void selectVariations();


#ifdef GEN_USE_COMP
	RE::ShaderProgram m_basicTerrainShader = RE::ShaderProgram{{.comp = basicTerrain_comp}};
	RE::ShaderProgram m_cellularAutomatonShader = RE::ShaderProgram{{.comp = cellularAutomaton_comp}};
	RE::ShaderProgram m_selectVariationShader = RE::ShaderProgram{{.comp = selectVariation_comp}};

	std::array<RE::Texture, 2> m_tilesTex = {
		RE::Texture{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}},
		RE::Texture{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}}
	};
	RE::Texture m_materialGenTex{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}};
#else
	RE::VertexArray m_VAO;
	RE::ShaderProgram m_basicTerrainShader = RE::ShaderProgram{{.vert = chunkGen_vert, .frag = basicTerrain_frag}};
	RE::ShaderProgram m_cellularAutomatonShader = RE::ShaderProgram{{.vert = chunkGen_vert, .frag = cellularAutomaton_frag}};
	RE::ShaderProgram m_selectVariationShader = RE::ShaderProgram{{.vert = chunkGen_vert, .frag = selectVariation_frag}};

	std::array<RE::Surface, 2> m_genSurf = {
		RE::Surface{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, 2, true, false},
		RE::Surface{{GEN_CHUNK_SIZE}, {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, 1, true, false}
	};
#endif
	int m_seed = 0;
};