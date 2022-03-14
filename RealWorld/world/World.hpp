#pragma once
#include <string>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/Surface.hpp>

#include <RealWorld/world/chunk/ChunkManager.hpp>
#include <RealWorld/world/WorldData.hpp>
#include <RealWorld/shaders/world_dynamics.hpp>
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

	glm::uvec2 adoptWorldData(const WorldData& wd, const std::string& name, const glm::vec2& windowDims);
	void gatherWorldData(WorldData& wd) const;
	bool saveChunks() const;
private:
	using enum RE::BufferType;
	using enum RE::BufferAccessFrequency;
	using enum RE::BufferMapUsageFlags;
	using enum RE::VertexComponentCount;
	using enum RE::VertexComponentType;
	using enum RE::Primitive;

	int m_seed = 0;
	glm::uvec2 m_activeChunksRect{16u, 16u};
	RE::Surface m_worldSurface = RE::Surface(RE::Raster{glm::uvec2(CHUNK_SIZE) * m_activeChunksRect}, {RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false);

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

	RE::ShaderProgram m_dynamicsShader = RE::ShaderProgram{{.comp = dynamics_comp}};
	RE::ShaderProgram m_transformShader = RE::ShaderProgram{{.comp = transform_comp}};
	RE::ShaderProgram m_modifyShader = RE::ShaderProgram{{.comp = modify_comp}};

	std::array<glm::ivec4, 4> m_dynamicsUpdateOrder = {glm::ivec4{0, 0, 0, 0}, glm::ivec4{1, 0, 1, 0}, glm::ivec4{0, 1, 0, 1}, glm::ivec4{1, 1, 1, 1}};
	uint32_t m_rngState;

	ChunkManager m_chunkManager;
};