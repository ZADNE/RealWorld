/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <string>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/textures/Texture.hpp>

#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/shaders/simulation.hpp>
#include <RealWorld/reserved_units/buffers.hpp>

enum class MODIFY_SHAPE : unsigned int {
	SQUARE,
	DISC,
	FILL
};

/**
 * @brief Represents the world as an endless grid of tiles.
 *
 * Also runs tile transformation and fluid dynamics simulation.
 */
class World {
public:

	/**
	 * @brief Initializes the world
	 * @param The generator that will be used to generate new chunks
	*/
	World(ChunkGenerator& chunkGen);

	/**
	 * @copydoc ChunkHandler::getNumberOfInactiveChunks
	*/
	size_t getNumberOfInactiveChunks();

	/**
	 * @brief Modifies tiles in the world
	*/
	void modify(LAYER layer, MODIFY_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile);

	/**
	 * @brief Performs a simulation step of the world.
	 * @param botLeftTi The most bottom-left tile that has to be active
	 * @param topRightTi The most top-right tile that has to be active
	 * @return The number of chunks that had to be activated this step
	*/
	int step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

	/**
	 * @brief Sets this world class to simulate the world inside the given save
	 * @param save Save of the world to run
	 * @param activeChunksArea Size of the main texture that holds active chunks. Measured in chunks, must be multiples of 8.
	*/
	void adoptSave(const MetadataSave& save, const glm::ivec2& activeChunksArea);

	void gatherSave(MetadataSave& save) const;

	bool saveChunks() const;

	void shouldPermuteOrder(bool should) { m_permuteOrder = should; }
private:
	void fluidDynamicsStep(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi);

	RE::Surface m_worldSrf;
	int m_seed = 0;

	std::string m_worldName;

	struct WorldDynamicsUBO {
		glm::ivec2 globalPosTi;
		glm::uint modifyTarget;
		glm::uint modifyShape;
		glm::uvec2 modifySetValue;
		float modifyDiameter;
		glm::uint timeHash;
		glm::ivec4 updateOrder[16];//Only the first two components are valid, the other two are padding required for std140 layout
	};
	RE::TypedBuffer m_worldDynamicsBuf{UNIF_BUF_WORLDDYNAMICS, sizeof(WorldDynamicsUBO), RE::BufferUsageFlags::MAP_WRITE};

	RE::ShaderProgram m_fluidDynamicsShd = RE::ShaderProgram{{.comp = fluidDynamics_comp}};
	RE::ShaderProgram m_tileTransformationsShd = RE::ShaderProgram{{.comp = tileTransformations_comp}};
	RE::ShaderProgram m_modifyShd = RE::ShaderProgram{{.comp = modify_comp}};

	std::array<glm::ivec4, 4> m_dynamicsUpdateOrder = {glm::ivec4{0, 0, 0, 0}, glm::ivec4{1, 0, 1, 0}, glm::ivec4{0, 1, 0, 1}, glm::ivec4{1, 1, 1, 1}};
	uint32_t m_rngState;

	ChunkManager m_chunkManager;

	bool m_permuteOrder = true;
};