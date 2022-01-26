#pragma once
#include <string>

#include <glm/mat4x4.hpp>

#include <RealEngine/graphics/VertexArray.hpp>
#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world/WorldDrawer.hpp>
#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/world/WorldData.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/furniture/FurnitureRegister.hpp>
#include <RealWorld/rendering/Vertex.hpp>


/**
 * Represents world as a grid of tiles.
 */
class World {
public:
	World(const glm::mat4& viewMatrix, const glm::uvec2& windowDims, RE::SpriteBatch& sb, Player& player);
	~World();

	/**
	 * Loads a world.
	 * Previously loaded world is flushed without saving.
	 *
	 * \param worldName Filename of the world
	 * \return True if successful, false otherwise. No change is done to curretn world then.
	 */
	bool loadWorld(const std::string& worldName);

	/**
	 * Save current world. Makes no changes to the world.
	 *
	 * \return True if successful, false otherwise.
	 */
	bool saveWorld() const;

	void resizeWindow(const glm::mat4& newViewMatrix, const glm::uvec2& newWindowDims);

	void switchDebugDraw() { m_shouldDrawDebug = !m_shouldDrawDebug; };
	void switchDebugDarkness() { m_shouldDrawDarkness = !m_shouldDrawDarkness; };


	//For adding and removing lights
	LightManipulator getLightManipulator();

	//Getters
	int getNumberOfChunksLoaded();

	glm::vec2 getGravity() {
		return m_gravity;
	};
	const std::string& getName() { return m_worldName; };


	//Tile functions
	uchar get(chunk::BLOCK_VALUES type, const glm::ivec2& posBc);
	uchar getMax(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc);
	uchar getMax(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc);
	uchar getMin(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc);
	uchar getMin(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc);

	void set(chunk::SET_TYPES type, const glm::ivec2& posBc, uchar index);

	bool exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc, uchar index);
	bool exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc, uchar index);

	//Furniture functions
	FIndex build(const FStatic& furniture, bool actuallyBuild = true);
	std::pair<size_t, glm::ivec2> destroy(const glm::ivec2& posSq);

	void deallocateAll();

	//All dynamic lights must be added each between beginStep() and endStep()
	void beginStep(const glm::vec2& middlePosPx, const glm::vec2& botLeftPosPx);
	//All dynamic lights must be added each between beginStep() and endStep()
	void endStep();

	//Draw step
	void drawBeginStep();
	void drawEndStep();

private:
	void adoptWorldData(const WorldData& wd, const std::string& name);
	void gatherWorldData(WorldData& wd) const;

	void drawDebug();

	int m_seed;
	glm::uvec2 m_chunkDims;
	glm::uvec2 m_activeChunksRect = glm::uvec2(6u, 6u);
	RE::Surface m_ws = RE::Surface({RE::TextureFlags::RGBA_IU_NEAR_NEAR_EDGE}, true, false);


	std::string m_folderName;
	std::string m_worldName;

	Player& m_player;
	WorldDrawer m_worldDrawer;
	glm::vec2 m_windowDims;

	glm::vec2 m_gravity = glm::vec2(0.0f, -0.5f);

	void initVAOs();
	void initConstantUniforms();
	void initUniformBuffers();

	void rebuildDebugRectangle();
	void updateUniformsAfterWorldResize();

	using enum RE::BufferUsageFlags;
	using enum RE::VertexComponentCount;
	using enum RE::VertexComponentType;

	//Set with update
	RE::VertexArray m_setWithUpdateVAO;
	RE::Buffer<RE::BufferType::ARRAY> m_setWithUpdateVBO{
		sizeof(SET_WITH_UPDATE_VERTICES), NO_FLAGS, SET_WITH_UPDATE_VERTICES
	};
	float m_time = 17.0f;
	RE::ShaderProgramPtr m_setWithUpdateShader = RE::RM::getShaderProgram({.vert = shaders::setWithUpdate_vert, .frag = shaders::setWithUpdate_frag});

	//Debug
	RE::ShaderProgramPtr m_debugDraw = RE::RM::getShaderProgram({.vert = shaders::data_vert, .frag = shaders::worldDebug_frag});
	RE::VertexArray m_debugVAO;
	RE::Buffer<RE::BufferType::ARRAY> m_debugVBO{
		sizeof(VertexPOUV) * 6, DYNAMIC_STORAGE
	};

	bool m_shouldDrawDebug = false;
	bool m_shouldDrawDarkness = true;

	FurnitureRegister m_fReg;
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