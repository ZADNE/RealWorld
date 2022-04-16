#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/default_shaders.hpp>
#include <magic_enum/magic_enum.hpp>

#include <RealWorld/items/Item.hpp>
#include <RealWorld/items/ItemSprite.hpp>
#include <RealWorld/shaders/common.hpp>

class ItemUser;
class Inventory;

/**
 * @brief Renders and manipulates invontories.
*/
class InventoryUI {
public:
	enum Connection { PRIMARY, SECONDARY, TERTIARY };
	enum class SelectionManner { ABS, RIGHT, LEFT, PREV, LAST_SLOT };

	/**
	 * @brief Contructs a UI that is not connected to any inventories
	*/
	InventoryUI(RE::SpriteBatch& spriteBatch, const glm::vec2& windowSize, const RE::FontSeed& font);
	~InventoryUI();

	/**
	 * @brief Notifies the UI that the window has been resized
	*/
	void windowResized(const glm::vec2& newWindowSize);

	/**
	 * @brief Connects the UI with given inventory (connection is mutual)
	 *
	 * Disconnects the previous inventory.
	 * @param inventory The inventory to connect with. nullptr effectively disconnects the previous inventory
	 * @param connection The type of conenction that is to be established
	*/
	void connectToInventory(Inventory* inventory, Connection connection);

	/**
	 * @brief Connects the UI with given item user
	 *
	 * Disconnects the previous item user.
	 * @param itemUser The item user to connect with. nullptr effectively disconnects the previous item user
	*/
	void connectToItemUser(ItemUser* itemUser) { m_itemUser = itemUser; }

	/**
	 * @brief Switches the inventory to the oposite state
	 *
	 * May be blocked if an item is held.
	*/
	void openOrClose();
	bool isOpen() { return m_open; }

	void reload();

	void swapUnderCursor(const glm::vec2& cursorPx);
	void movePortion(const glm::vec2& cursorPx, float portion);
	//Try to choose a number (may fail if tried to choose a slot outside inventory)
	//For some constants, number is irrelevant (e.g. LAST_SLOT, PREV)

	/**
	 * @brief Selects a slot from the first row of the inventory.
	 *
	 * The item in the selected slot is used by connected item user.
	 * @param selectionManner Specifies interpretation of number parameter
	 * @param number The slot to select
	*/
	void selectSlot(SelectionManner selectionManner, int number);

	void step();
	void draw(const glm::vec2& cursorPx);
private:
	static constexpr glm::vec2 SLOT_PADDING = glm::vec2(8.0f, 8.0f);

	inline glm::vec2 slotDims() const { return m_slotTex->getSubimageDims(); }
	inline glm::vec2 slotPivot() const { return m_slotTex->getPivot(); }

	inline glm::ivec2 invSize(Connection con) const;
	inline int invSlotCount(Connection con) const;

	std::optional<glm::ivec2> cursorToSlot(const glm::vec2& cursorPx) const;

	template<typename Func>
	void forEachConnectedInventory(Func f) {
		for (int c = PRIMARY; c < magic_enum::enum_count<Connection>(); ++c) {
			if (m_inv[c]) {//If connected the inventory
				f(static_cast<Connection>(c));
			}
		}
	}

	template<typename Func>
	void forEachSlotByIndex(Connection con, Func f) {
		for (int i = 0; i < invSlotCount(con); ++i) {
			f(i);
		}
	}

	template<typename Func>
	void forEachSlotByPosition(Connection con, Func f) {
		for (int y = 0; y < invSize(con).y; y++) {
			for (int x = 0; x < invSize(con).x; x++) {
				f(x, y);
			}
		}
	}

	RE::SpriteBatch& m_spriteBatch;
	RE::FontSeed m_font;
	ItemUser* m_itemUser = nullptr;

	glm::vec2 m_windowSize;
	RE::TexturePtr m_slotTex = RE::RM::getTexture("slot");
	glm::vec2 m_invBotLeftPx; /**< Bottom left corner of slot (0, 0) */

	Item m_heldItem{};
	ItemSprite m_heldSprite{};

	int m_chosenSlot = 0;//Is signed but never should be negative
	int m_chosenSlotPrev = 0;//Is signed but never should be negative

	RE::Surface m_slotsSurf{{RE::TextureFlags::RGBA8_NU_NEAR_NEAR_EDGE}, true, false};
	//0 texture: below dynamic sprites
	//1 texture: above dynamic sprites

	RE::Color m_amountColor{255u, 255u, 255u, 255u};

	Inventory* m_inv[magic_enum::enum_count<Connection>()] = {nullptr, nullptr, nullptr};
	std::vector<ItemSprite> m_invItemSprites[magic_enum::enum_count<Connection>()];
	bool m_open = false;
};