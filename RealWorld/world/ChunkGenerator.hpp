#pragma once
#include <array>

#include <RealEngine/graphics/ShaderProgram.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/world/Chunk.hpp>
#include <RealWorld/shaders/chunk_generation.hpp>
#include <RealWorld/rendering/UniformBuffers.hpp>

/**
 * Generates new chunks.
 */
class ChunkGenerator {
public:

	/**
	 * Contructs new chunk generator, initializes world-independent members.
	 */
	ChunkGenerator();

	/**
	 * Decontructs chunk generator, frees all memory.
	 */
	~ChunkGenerator();

	/**
	 * Prepares generator to generate chunks for given world
	 *
	 * \param seed Seed of the world
	 * \param chunkDimsTi Chunk dimensions of the world
	 */
	void setTargetWorld(int seed, glm::uvec2 chunkDimsTi);

	/**
	 * Generates single chunk. The pixels are stored to given texture at given position
	 * and also returned as a Chunk.
	 *
	 * \param posCh Position of the chunk (measured in chunks)
	 * \param uploadTexture Name of texture that will receive the generated chunk
	 * \param uploadOffset Offset within uploadTexture where the pixels will be copied
	 * \return Newly generated chunk
	 */
	Chunk generateChunk(glm::ivec2 posCh, GLuint uploadTexture, glm::ivec2 uploadOffset);

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
		glm::mat4 chunkGenMatrix;
		glm::vec2 chunkOffsetTi;
		glm::vec2 chunkDimsTi;
		glm::vec2 chunkBorders;
		float seed;
	};
	RE::UniformBuffer m_chunkUniformBuffer{UNIF_BUF_CHUNKGEN, true, RE::BufferUsageFlags::DYNAMIC_STORAGE, sizeof(ChunkUniforms)};

	void initShaders();
	void initVAO();
	void setVBOToWholeChunk();
	void updateUniformsAfterSetTarget();

	//Actual generation functions
	void generateBasicTerrain();
	void cellularAutomaton();
	void selectVariations();

	RE::VertexArray m_VAO;
	RE::Buffer<ARRAY, IMMUTABLE> m_VBO{sizeof(RE::VertexPO) * 4, DYNAMIC_STORAGE};

	RE::ShaderProgram m_basicTerrainShader = RE::ShaderProgram{{.vert = chunkGen_vert, .frag = basicTerrain_frag}};
	RE::ShaderProgram m_cellularAutomatonShader = RE::ShaderProgram{{.vert = chunkGen_vert, .frag = cellularAutomaton_frag}};
	RE::ShaderProgram m_selectVariationShader = RE::ShaderProgram{{.vert = chunkGen_vert, .frag = selectVariation_frag}};

	std::array<RE::Surface, 2> m_genSurf = {
		RE::Surface{{RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false},
		RE::Surface{{RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false}
	};
	glm::vec2 m_chunkDimsTi;
	int m_seed = 0;
};