/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/main/Room.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealEngine/graphics/View.hpp>
#include <RealEngine/graphics/Viewport.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/WorldDrawer.hpp>
#include <RealWorld/physics/Player.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/InventoryUI.hpp>

const unsigned int FPS_LIMIT = 300u;
const glm::vec4 SKY_BLUE = glm::vec4(0.25411764705f, 0.7025490196f, 0.90470588235f, 1.0f);

/**
 * @brief Holds all gameplay-related objects.
*/
class WorldRoom : public Room {
public:
	WorldRoom(RE::CommandLineArguments args);
	~WorldRoom();
	virtual void sessionStart(const RE::RoomTransitionParameters& params) override;
	virtual void sessionEnd() override;
	virtual void step() override;
	virtual void render(double interpolationFactor) override;

	virtual const RE::RoomDisplaySettings& getDisplaySettings() override {
		static RE::RoomDisplaySettings settings{
			.clearColor = SKY_BLUE,
			.stepsPerSecond = PHYSICS_STEPS_PER_SECOND,
			.framesPerSecondLimit = FPS_LIMIT,
			.usingImGui = true
		};
		return settings;
	}

	virtual void windowResized(const glm::ivec2& newDims) override;
private:

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

	ImFont* m_arial = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20.0f);

	//View
	RE::View m_worldView{window()->getDims()};
	RE::UniformBuffer m_worldViewUBO{RE::UNIF_BUF_VIEWPORT_MATRIX, false, sizeof(glm::mat4), RE::BufferUsageFlags::DYNAMIC_STORAGE};

	//Gameplay
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