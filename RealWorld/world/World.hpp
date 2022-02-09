﻿#pragma once
#include <string>

#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/VertexArray.hpp>
#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/UniformBuffer.hpp>

#include <RealWorld/world/chunk/ChunkManager.hpp>
#include <RealWorld/world/WorldData.hpp>
#include <RealWorld/shaders/world_dynamics.hpp>
#include <RealWorld/rendering/Vertex.hpp>

/**
 * Represents world as a grid of tiles.
 */
class World {
public:
	World();
	~World();

	//Getters
	int getNumberOfChunksLoaded();

	int addGravityEveryNSteps() { return 2; };
	const std::string& getName() { return m_worldName; };

	/**
	 * @copydoc ChunkHandler::forceActivationOfChunks
	*/
	void forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

	//Tile functions
	uchar get(TILE_VALUE type, const glm::ivec2& posTi);
	uchar getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi);
	uchar getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);
	uchar getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi);
	uchar getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

	void set(SET_TYPES type, const glm::ivec2& posTi, uchar index);

	bool exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi, uchar index);
	bool exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi, uchar index);

	void step();

	glm::uvec2 adoptWorldData(const WorldData& wd, const std::string& name, const glm::vec2& windowDims);
	void gatherWorldData(WorldData& wd) const;
	bool saveChunks() const;
private:
	using enum RE::BufferType;
	using enum RE::BufferStorage;
	using enum RE::BufferAccessFrequency;
	using enum RE::BufferAccessNature;
	using enum RE::BufferUsageFlags;
	using enum RE::VertexComponentCount;
	using enum RE::VertexComponentType;
	using enum RE::Primitive;

	int m_seed;
	glm::uvec2 m_chunkDims;
	glm::uvec2 m_activeChunksRect = glm::uvec2(32u, 32u);
	RE::Surface m_ws = RE::Surface({RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false);

	std::string m_worldName;

	void initVAOs();
	void initUniformBuffers();

	void updateUniformsAfterWorldResize();

	//Set with update
	RE::VertexArray m_setWithUpdateVAO;
	RE::Buffer<ARRAY> m_setWithUpdateVBO{
		sizeof(SET_WITH_UPDATE_VERTICES), NO_FLAGS, SET_WITH_UPDATE_VERTICES
	};
	float m_time = 17.0f;
	RE::ShaderProgram m_setWithUpdateShader = RE::ShaderProgram{{.vert = setWithUpdate_vert, .frag = setWithUpdate_frag}};

	struct WorldUniforms {
		glm::mat4 worldMatrix;
		glm::vec2 worldSize;
	};
	RE::UniformBuffer m_worldUniformBuffer{UNIF_BUF_WORLD, true, RE::BufferUsageFlags::DYNAMIC_STORAGE, sizeof(WorldUniforms)};

	ChunkManager m_chunkManager;

	const unsigned int ATTR_SET_AROUND = 1u;

	struct VertexSetWithUpdate {
		VertexSetWithUpdate(const glm::vec2& position, unsigned int setAround) :
			position(position), setAround(setAround) {}

		glm::vec2 position;
		unsigned int setAround;//bitfield informing about blocks added/removed around this block
	};

	inline static const VertexSetWithUpdate SET_WITH_UPDATE_VERTICES[9] = {
		{{-1.0f, -1.0f}, 16}, {{-1.0f, 0.0f}, 32}, {{-1.0f, 1.0f}, 64},
		{{0.0f, -1.0f}, 8},  {{0.0f, 0.0f}, 0},  {{0.0f, 1.0f}, 128},
		{{1.0f, -1.0f}, 4},  {{1.0f, 0.0f}, 2},  {{1.0f, 1.0f}, 1}
	};
};