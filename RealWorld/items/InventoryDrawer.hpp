#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/default_shaders.hpp>

#include <RealWorld/world/physics/Hitbox.hpp>
#include <RealWorld/items/Item.hpp>
#include <RealWorld/items/ItemSprite.hpp>
#include <RealWorld/shaders/common.hpp>

enum Connection { PRIMARY, SECONDARY, TERTIARY, NUMBER_OF_TYPES };

enum class Choose { ABS, RIGHT, LEFT, PREV, LAST_SLOT };

class Inventory;
class ItemUser;

class InventoryDrawer {
public:
	InventoryDrawer(RE::SpriteBatch& spriteBatch, const glm::vec2& windowSize, const RE::FontSeed& font);
	~InventoryDrawer();

	void resizeWindow(const glm::vec2& newWindowSize);
	//nullptr effectively disconnects previous inventory
	//Inventory is also connected to this drawer (connection must be mutual to work properly)
	void connectToInventory(Inventory* inventory, Connection connection);
	//nullptr effectively disconnects previous ItemUser
	void connectToItemUser(ItemUser* itemUser);
	//Atempt to switch state (opening or closing inventory)
	//May be blocked by many things, such as holding item under cursor)
	void switchState();
	bool isOpen();

	void reloadNumbers();
	void reloadEverything();

	void swapUnderCursor();
	void movePortion(float portion);
	//Try to choose a number (may fail if tried to choose a slot outside inventory)
	//For some constants, number is irrelevant (e.g. LAST_SLOT, PREV)
	void chooseSlot(Choose chooseConst, int number);

	void step(const glm::ivec2& absCursorPos);
	void draw();
private:
	RE::SpriteBatch& m_spriteBatch;
	RE::FontSeed m_font;
	ItemUser* m_itemUser = nullptr;

	RE::ShaderProgram m_PTSAbove{{.vert = RE::sprite_vert, .frag = standardOut1_frag }};

	RE::Colour m_defColour{255, 255, 255, 255};
	glm::vec2 m_windowSize;
	RE::TexturePtr m_mainSlotTex = RE::RM::getTexture("mainSlot");
	RE::TexturePtr m_slotIndicatorTex = RE::RM::getTexture("slotIndicator");
	glm::ivec2 m_mainSlotDims = m_mainSlotTex->getSubimageDims();
	glm::ivec2 m_paddingSlots = glm::vec2(8, 8);
	glm::ivec2 m_paddingWindow;//Center of [0;0] slot of main inventory

	glm::ivec2 m_absCursorPos;//Absolute position of the cursor in the window
	Item m_itemUnderCursor{};
	ItemSprite m_underCursorItemSprite{};

	std::vector<std::vector<Hitbox>> m_hitboxes;
	Hitbox m_hitboxMainCover{glm::ivec2{0, 0}, glm::ivec2{0, 0}, m_mainSlotDims};
	int m_chosenSlot = 0;//Is signed but never should be negative
	int m_chosenSlotPrev = 0;//Is signed but never should be negative

	RE::Surface m_cover{{RE::TextureFlags::RGBA8_NU_NEAR_NEAR_EDGE}, true, false};
	//0 texture: below dynamic sprites
	//1 texture: above dynamic sprites

	RE::Colour m_amountColour{150u, 75u, 0u, 255u};

	void updateHitboxMesh();
	void updateSurfaceSlots();
	void updateSurfaceNumbers();

	Inventory* m_inv[(size_t)Connection::NUMBER_OF_TYPES] = {nullptr, nullptr, nullptr};
	glm::ivec2 m_invSize[(size_t)Connection::NUMBER_OF_TYPES];
	std::vector<std::vector<ItemSprite>> m_invItemSprites[(size_t)Connection::NUMBER_OF_TYPES];
	bool m_opened = false;
	bool m_shouldDraw = true;
};