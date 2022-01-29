#pragma once
#include <array>

#include <RealEngine/graphics/ShaderProgram.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/VertexArray.hpp>

#include <RealWorld/world/Chunk.hpp>
#include <RealWorld/shaders/shaders.hpp>
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
	 * \param chunkDims Chunk dimensions of the world
	 * \param activeChunksRect Active chunk rectangle of the world
	 */
	void setTargetWorld(int seed, glm::uvec2 chunkDims, glm::uvec2 activeChunksRect);

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
		glm::vec2 chunkOffsetBc;
		glm::vec2 chunkDims;
		glm::vec2 chunkBorders;
		int seed;
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

	RE::ShaderProgram m_basicTerrainShader = RE::ShaderProgram{{.vert = WGS::chunkGen_vert, .frag = WGS::basicTerrain_frag}};
	RE::ShaderProgram m_cellularAutomatonShader = RE::ShaderProgram{{.vert = WGS::chunkGen_vert, .frag = WGS::cellularAutomaton_frag}};
	RE::ShaderProgram m_selectVariationShader = RE::ShaderProgram{{.vert = WGS::chunkGen_vert, .frag = WGS::selectVariation_frag}};

	std::array<RE::Surface, 2> m_genSurf = {
		RE::Surface{ {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false },
		RE::Surface{ {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false }};
	glm::uvec2 m_chunkDims;
	glm::vec2 m_chunkDims_f;
	glm::ivec2 m_activeChunksRect;
	int m_seed = 0;
};