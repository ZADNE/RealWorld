#pragma once
#include <memory>

#include <RealEngine/main/Room.hpp>
#include <RealEngine/graphics/View.hpp>
#include <RealEngine/graphics/Viewport.hpp>
#include <RealEngine/resources/FontCache.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/WorldDrawer.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/furniture/FurnitureManager.hpp>
#include <RealWorld/items/InventoryDrawer.hpp>
#include <RealWorld/items/CraftingDrawer.hpp>
#include <RealWorld/items/ItemOnGroundManager.hpp>

class WorldRoom : public RE::Room {
public:
	WorldRoom(RE::CommandLineArguments args);
	~WorldRoom();
	virtual void sessionStart(const RE::RoomTransitionParameters& params) override;
	virtual void sessionEnd() override;
	virtual void step() override;
	virtual void render(double interpolationFactor) override;

	void resizeWindow(const glm::ivec2& newDims, bool isPermanent);
private:
	void drawGUI();

	/**
	 * @brief Loads a world.
	 * Previously loaded world is flushed without saving.
	 *
	 * @param worldName Filename of the world
	 * @return True if successful, false otherwise. No change is done to current world then.
	 */
	bool loadWorld(const std::string& worldName);

	/**
	 * Save current world. Makes no changes to the world.
	 *
	 * \return True if successful, false otherwise.
	 */
	bool saveWorld() const;

	//View
	RE::View m_worldView{window()->getDims()};
	RE::UniformBuffer m_worldViewUnifromBuffer{RE::UNIF_BUF_VIEWPORT_MATRIX, false, RE::BufferUsageFlags::DYNAMIC_STORAGE, sizeof(glm::mat4)};

	//Standard drawing
	RE::FontSeed m_inventoryFont{"arial", 24, 32, 350};

	//Gameplay
	World m_world;
	WorldDrawer m_worldDrawer;
	FurnitureManager m_furnitureManager;
	Player m_player;
	InventoryDrawer m_inventoryDrawer;
	CraftingDrawer m_craftingDrawer;
	ItemOnGroundManager m_itemOnGroundManager;
};