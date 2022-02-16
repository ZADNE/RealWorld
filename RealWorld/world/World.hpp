#pragma once
#include <string>

#include <RealEngine/graphics/VertexArray.hpp>
#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/UniformBuffer.hpp>

#include <RealWorld/world/chunk/ChunkManager.hpp>
#include <RealWorld/world/WorldData.hpp>
#include <RealWorld/shaders/world_dynamics.hpp>
#include <RealWorld/rendering/Vertex.hpp>

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
	 * @copydoc ChunkHandler::getNumberOfChunksLoaded
	*/
	size_t getNumberOfChunksLoaded();

	float gravity() { return 0.1875f; };

	//Tile functions
	uchar get(TILE_VALUE type, const glm::ivec2& posTi);
	uchar getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi);
	uchar getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);
	uchar getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi);
	uchar getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

	void set(SET_TARGET target, SET_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile);

	bool exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi, uchar index);
	bool exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi, uchar index);

	void step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

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

	int m_seed = 0;
	glm::uvec2 m_activeChunksRect{16u, 16u};
	RE::Surface m_worldSurface = RE::Surface(RE::Raster{glm::uvec2(CHUNK_SIZE) * m_activeChunksRect}, {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false);

	std::string m_worldName;

	void updateUniformsAfterWorldResize();

	//Set with update
	RE::VertexArray m_setWithUpdateVAO;
	RE::Buffer<ARRAY> m_setWithUpdateVBO{
		sizeof(SET_WITH_UPDATE_VERTICES), NO_FLAGS, SET_WITH_UPDATE_VERTICES
	};
	RE::ShaderProgram m_setWithUpdateShader = RE::ShaderProgram{{.vert = setWithUpdate_vert, .frag = setWithUpdate_frag}};
	RE::ShaderProgram m_dynamicsShader = RE::ShaderProgram{{.comp = dynamics_comp}};
	RE::ShaderProgram m_modifyShader = RE::ShaderProgram{{.comp = modify_comp}};

	std::array<glm::ivec2, 4> m_dynamicsOrder = {glm::ivec2{0, 0}, glm::ivec2{1, 0}, glm::ivec2{0, 1}, glm::ivec2{1, 1}};
	uint32_t m_rngState;

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