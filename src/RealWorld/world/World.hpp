﻿#pragma once
#include <string>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/Surface.hpp>

#include <RealWorld/chunk/ChunkManager.hpp>
#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/shaders/simulation.hpp>
#include <RealWorld/rendering/Vertex.hpp>
#include <RealWorld/rendering/UniformBuffers.hpp>

enum class SET_SHAPE : unsigned int {
	SQUARE,
	DISC,
	FILL
};

/**
 * Represents world as an endless grid of tiles.
 */
class World {
public:
	World();
	~World();

	/**
	 * @copydoc ChunkHandler::getNumberOfInactiveChunks
	*/
	size_t getNumberOfInactiveChunks();

	float gravity() { return 0.1875f; };

	void set(SET_TARGET target, SET_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile);

	void step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

	glm::uvec2 adoptSave(const MetadataSave& save, const glm::vec2& windowDims);
	void gatherSave(MetadataSave& save) const;
	bool saveChunks() const;
private:
	using enum RE::BufferType;
	using enum RE::BufferAccessFrequency;
	using enum RE::BufferMapUsageFlags;
	using enum RE::VertexComponentCount;
	using enum RE::VertexComponentType;
	using enum RE::Primitive;

	int m_seed = 0;
	RE::Surface m_worldSurface = RE::Surface(RE::Raster{iCHUNK_SIZE * ACTIVE_CHUNKS_AREA}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}, true, false);

	std::string m_worldName;

	struct WorldDynamicsUBO {
		glm::ivec2 globalPosTi;
		glm::uint modifyTarget;
		glm::uint modifyShape;
		glm::uvec2 modifySetValue;
		float modifyDiameter;
		glm::uint timeHash;
		glm::ivec4 updateOrder[16];//Only first two components are valid, second two are padding
	};
	RE::UniformBuffer m_worldDynamicsUBO{UNIF_BUF_WORLDDYNAMICS, true, sizeof(WorldDynamicsUBO), RE::BufferUsageFlags::MAP_WRITE};

	RE::ShaderProgram m_fluidDynamicsShader = RE::ShaderProgram{{.comp = fluidDynamics_comp}};
	RE::ShaderProgram m_tileTransformationsShader = RE::ShaderProgram{{.comp = tileTransformations_comp}};
	RE::ShaderProgram m_modifyShader = RE::ShaderProgram{{.comp = modify_comp}};

	std::array<glm::ivec4, 4> m_dynamicsUpdateOrder = {glm::ivec4{0, 0, 0, 0}, glm::ivec4{1, 0, 1, 0}, glm::ivec4{0, 1, 0, 1}, glm::ivec4{1, 1, 1, 1}};
	uint32_t m_rngState;

	ChunkManager m_chunkManager{m_tileTransformationsShader, 0u};
};