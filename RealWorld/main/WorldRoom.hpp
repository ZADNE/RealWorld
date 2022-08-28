/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/main/Room.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealEngine/rendering/cameras/View2D.hpp>
#include <RealEngine/rendering/output/Viewport.hpp>

#include <RealWorld/main/settings/GameSettings.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/drawing/WorldDrawer.hpp>
#include <RealWorld/physics/Player.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/InventoryUI.hpp>

 //Use this to swap generators
#define FBO_GENERATOR 1
#define CS_GENERATOR 2
#define CHUNK_GENERATOR CS_GENERATOR
#if CHUNK_GENERATOR == CS_GENERATOR
#include <RealWorld/generation/ChunkGeneratorCS.hpp>
#elif CHUNK_GENERATOR == FBO_GENERATOR
#include <RealWorld/generation/ChunkGeneratorFBO.hpp>
#else
#error "No chunk generator has been selected!"
#endif

#ifdef _DEBUG
constexpr unsigned int FPS_LIMIT = 300u;
#else
constexpr unsigned int FPS_LIMIT = RE::Synchronizer::DO_NOT_LIMIT_FRAMES_PER_SECOND;
#endif // _DEBUG

constexpr glm::vec4 SKY_BLUE = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

/**
 * @brief Holds all gameplay-related objects.
*/
class WorldRoom : public Room {
public:
	WorldRoom(const GameSettings& gameSettings);

	void sessionStart(const RE::RoomTransitionParameters& params) override;
	void sessionEnd() override;
	void step() override;
	void render(double interpolationFactor) override;

	void windowResizedCallback(const glm::ivec2& oldSize, const glm::ivec2& newSize) override;

private:
	static constexpr RE::RoomDisplaySettings DEFAULT_SETTINGS{
		.clearColor = SKY_BLUE,
		.stepsPerSecond = PHYSICS_STEPS_PER_SECOND,
		.framesPerSecondLimit = FPS_LIMIT,
		.usingImGui = true
	};

	void drawGUI();

	/**
	 * @brief Loads a world. Previously loaded world is flushed without saving.
	 *
	 * @param worldName Filename of the world
	 * @return True if successful, false otherwise. No change is done to current world then.
	 */
	bool loadWorld(const std::string& worldName);

	/**
	 * @brief Saves the current world. Makes no changes to the world.
	 *
	 * @return True if successful, false otherwise.
	 */
	bool saveWorld() const;

	const GameSettings& m_gameSettings;
	ImFont* m_arial = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20.0f);

	//View
	RE::View2D m_worldView{engine().getWindowDims()};
	RE::TypedBuffer m_worldViewUBO{RE::UNIF_BUF_VIEWPORT_MATRIX, RE::BindNow::NO, sizeof(glm::mat4), RE::BufferUsageFlags::DYNAMIC_STORAGE};

	//Gameplay
#if CHUNK_GENERATOR == CS_GENERATOR
	ChunkGeneratorCS m_chunkGen;
#elif CHUNK_GENERATOR == FBO_GENERATOR
	ChunkGeneratorFBO m_chunkGen;
#endif
	World m_world;
	WorldDrawer m_worldDrawer;
	Player m_player;
	Inventory m_playerInv;
	ItemUser m_itemUser;
	InventoryUI m_invUI;

	//Toggle states
	bool m_minimap = false;
	bool m_shadows = true;
	bool m_permute = true;
};