#pragma once
#include <memory>

#include <RealEngine/main/Room.hpp>
#include <RealEngine/graphics/View.hpp>
#include <RealEngine/resources/FontCache.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/world/physics/Player.hpp>
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

	//Views
	RE::View m_worldView;

	//Standard drawing
	RE::FontSeed m_inventoryFont{"arial", 24, 32, 350};

	//Gameplay
	World m_world;
	Player m_player;
	InventoryDrawer m_inventoryDrawer;
	CraftingDrawer m_craftingDrawer;
	ItemOnGroundManager m_itemOnGroundManager;
};