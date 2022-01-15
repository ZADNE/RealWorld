#pragma once
#include <random>
#include <array>

#include <RealEngine/ResourceManager.hpp>
#include <RealEngine/Surface.hpp>

#include "Chunk.hpp"
#include "../shaders/shaders.hpp"

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
	void initShaders();
	void initObjects();
	void setVBOToWholeChunk();
	void updateUniformsAfterSetTarget();

	//Actual generation functions
	void generateBasicTerrain();
	void cellularAutomaton();
	void setVars();

	GLuint m_VAO = 0;
	GLuint m_VBO = 0;

	RE::ShaderProgramPtr m_setShader = RE::RM::getShaderProgram({{.vert = WGS::chunkGen_vert, .frag = WGS::set_frag }});
	RE::ShaderProgramPtr m_basicTerrainShader = RE::RM::getShaderProgram({{.vert = WGS::chunkGen_vert, .frag = WGS::basicTerrain_frag }});
	RE::ShaderProgramPtr m_varShader = RE::RM::getShaderProgram({{.vert = WGS::chunkGen_vert, .frag = WGS::var_frag }});
	RE::ShaderProgramPtr m_cellularAutomatonShader = RE::RM::getShaderProgram({{.vert = WGS::chunkGen_vert, .frag = WGS::cellularAutomaton_frag }});

	std::mt19937 m_randomGen;

	std::array<RE::Surface, 2> m_genSurf = {
		RE::Surface{ {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false },
		RE::Surface{ {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false }};
	glm::uvec2 m_chunkDims;
	glm::vec2 m_chunkDims_f;
	glm::ivec2 m_activeChunksRect;
	int m_seed = 0;
};