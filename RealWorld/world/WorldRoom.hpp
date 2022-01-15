#pragma once
#include <memory>

#include <RealEngine/Room.hpp>
#include <RealEngine/View.hpp>
#include <RealEngine/FontCache.hpp>
#include <RealEngine/default_shaders.hpp>

class View;
class World;
class Player;
class InventoryDrawer;
class ItemOnGroundManager;
class CraftingDrawer;

class WorldRoom : public RE::Room {
public:
	WorldRoom();
	~WorldRoom();

	virtual void E_build(const std::vector<std::string>& buildArgs) override;
	virtual void E_destroy() override;
	virtual void E_entry(std::vector<void*> enterPointers) override;
	virtual std::vector<void*> E_exit() override;
	virtual void E_step() override;
	virtual void E_draw(double interpolationFactor) override;

	virtual int getNextIndex() const override;
	virtual int getPrevIndex() const override;

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