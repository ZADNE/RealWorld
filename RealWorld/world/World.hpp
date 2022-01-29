#pragma once
#include <string>

#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/VertexArray.hpp>
#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/UniformBuffer.hpp>

#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/world/WorldData.hpp>
#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/rendering/Vertex.hpp>

/**
 * Represents world as a grid of tiles.
 */
class World {
public:
	World();
	~World();

	void resizeWindow(const glm::vec2& newWindowDims);

	void switchDebugDraw() { m_shouldDrawDebug = !m_shouldDrawDebug; };

	//Getters
	int getNumberOfChunksLoaded();

	glm::vec2 getGravity() { return glm::vec2(0.0f, -0.5f); };
	const std::string& getName() { return m_worldName; };

	/**
	 * @copydoc ChunkHandler::forceActivationOfChunks
	*/
	void forceActivationOfChunks(const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc);

	//Tile functions
	uchar get(chunk::BLOCK_VALUES type, const glm::ivec2& posBc);
	uchar getMax(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc);
	uchar getMax(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc);
	uchar getMin(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc);
	uchar getMin(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc);

	void set(chunk::SET_TYPES type, const glm::ivec2& posBc, uchar index);

	bool exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc, uchar index);
	bool exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc, uchar index);

	void step();

	void drawDebug();

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
	glm::uvec2 m_activeChunksRect = glm::uvec2(6u, 6u);
	RE::Surface m_ws = RE::Surface({RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false);

	std::string m_worldName;

	void initVAOs();
	void initConstantUniforms();
	void initUniformBuffers();

	void rebuildDebugRectangle(const glm::vec2& newWindowDims);
	void updateUniformsAfterWorldResize();

	//Set with update
	RE::VertexArray m_setWithUpdateVAO;
	RE::Buffer<ARRAY> m_setWithUpdateVBO{
		sizeof(SET_WITH_UPDATE_VERTICES), NO_FLAGS, SET_WITH_UPDATE_VERTICES
	};
	float m_time = 17.0f;
	RE::ShaderProgram m_setWithUpdateShader = RE::ShaderProgram{{.vert = shaders::setWithUpdate_vert, .frag = shaders::setWithUpdate_frag}};

	//Debug
	RE::ShaderProgram m_debugDraw = RE::ShaderProgram{{.vert = shaders::data_vert, .frag = shaders::worldDebug_frag}};
	RE::VertexArray m_debugVAO;
	RE::Buffer<ARRAY> m_debugVBO{
		sizeof(VertexPOUV) * 4, DYNAMIC_STORAGE
	};

	struct WorldUniforms {
		glm::mat4 worldMatrix;
		glm::vec2 worldSize;
	};
	RE::UniformBuffer m_worldUniformBuffer{UNIF_BUF_WORLD, true, RE::BufferUsageFlags::DYNAMIC_STORAGE, sizeof(WorldUniforms)};

	bool m_shouldDrawDebug = false;

	ChunkManager m_chunkHandler;

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