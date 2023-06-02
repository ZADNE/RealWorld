/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/graphics/batches/SpriteBatch.hpp>

#include <RealWorld/items/Item.hpp>
#include <RealWorld/items/ItemSprite.hpp>

class ItemUser;
class Inventory;

/**
 * @brief Renders and manipulates inventories.
*/
class InventoryUI {
public:

    enum class SlotSelectionManner {
        AbsolutePos,
        ScrollRight,
        ScrollLeft,
        ToPrevious,
        ToLastSlot
    };

    enum Connection {
        Primary,
        Secondary,
        Tertiary,
        Count
    };

    /**
     * @brief Contructs a UI that is not connected to any inventories
    */
    InventoryUI(const glm::vec2& windowSize);

    InventoryUI(const InventoryUI&) = delete;           /**< Noncopyable */
    InventoryUI& operator=(const InventoryUI&) = delete;/**< Noncopyable */

    InventoryUI(InventoryUI&&) = delete;                /**< Nonmovable */
    InventoryUI&& operator=(InventoryUI&&) = delete;    /**< Nonmovable */

    ~InventoryUI();

    /**
     * @brief Notifies the UI that the window has been resized
    */
    void windowResized(const glm::vec2& newWindowSize);

    /**
     * @brief Connects the UI with given inventory (connection is mutual)
     * @param inventory The inventory to connect with. nullptr effectively disconnects the previous inventory
     * @param connection The type of conenction that is to be established
     *
     * Simultaneously disconnects the previous inventory.
    */
    void connectToInventory(Inventory* inventory, Connection connection);

    /**
     * @brief Connects the UI with given item user (disconnects the previous)
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

    /**
     * @brief Selects a slot from the first row of the inventory.
     *
     * The item in the selected slot is used by connected item user.
     * @param selectionManner Specifies interpretation of number parameter
     * @param number The slot to select (for some manners irrelevant)
    */
    void selectSlot(SlotSelectionManner selectionManner, int number = 0);

    void step();
    void draw(re::SpriteBatch& spriteBatch, const glm::vec2& cursorPx);

private:

    static constexpr glm::vec2 k_slotPadding = glm::vec2(8.0f, 8.0f);

    inline glm::vec2 slotDims() const { return m_slotTex.subimageDims(); }
    inline glm::vec2 slotPivot() const { return m_slotTex.pivot(); }

    inline glm::ivec2 invSize(Connection con) const;
    inline int invSlotCount(Connection con) const;

    std::optional<glm::ivec2> cursorToSlot(const glm::vec2& cursorPx) const;

    template<typename Func>
    void forEachConnectedInventory(Func f) {
        for (int c = Primary; c < static_cast<int>(Connection::Count); ++c) {
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

    ItemUser* m_itemUser = nullptr;

    glm::vec2 m_windowSize;
    re::TextureShaped m_slotTex{{.file = "slot"}};
    glm::vec2 m_invBotLeftPx; /**< Bottom left corner of slot (0, 0) */

    Item m_heldItem{};
    ItemSprite m_heldSprite{};

    int m_chosenSlot = 0;//Is signed but never should be negative
    int m_chosenSlotPrev = 0;//Is signed but never should be negative

    Inventory* m_inv[static_cast<size_t>(Connection::Count)] = {nullptr, nullptr, nullptr};
    std::vector<ItemSprite> m_invItemSprites[static_cast<size_t>(Connection::Count)];
    bool m_open = false;
};