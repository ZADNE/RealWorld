#pragma once
#include <memory>

#include <RealEngine/main/Room.hpp>
#include <RealEngine/graphics/View.hpp>
#include <RealEngine/resources/FontCache.hpp>
#include <RealEngine/graphics/default_shaders.hpp>

class View;
class World;
class Player;
class InventoryDrawer;
class ItemOnGroundManager;
class CraftingDrawer;

class WorldRoom : public RE::Room {
public:
	WorldRoom(RE::CommandLineArguments args);
	~WorldRoom();
	virtual void E_entry(RE::RoomTransitionParameters params) override;
	virtual RE::RoomTransitionParameters E_exit() override;
	virtual void E_step() override;
	virtual void E_draw(double interpolationFactor) override;

	void resizeWindow(const glm::ivec2& newDims, bool isPermanent);
private:
	void drawGUI();

	//Views
	std::unique_ptr<RE::View> m_worldView;

	//Standard drawing
	RE::FontSeed m_inventoryFont{"arial", 24, 32, 350};
	RE::FontSeed m_menuFont{"arial", 22, 32, 350};

	//Gameplay
	std::unique_ptr<World> m_world;
	std::unique_ptr<Player> m_player;
	std::unique_ptr<InventoryDrawer> m_inventoryDrawer;
	std::unique_ptr<CraftingDrawer> m_craftingDrawer;
	std::unique_ptr<ItemOnGroundManager> m_itemOnGroundManager;
};