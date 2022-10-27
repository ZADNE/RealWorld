/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>

#include <RealWorld/items/Item.hpp>
#include <RealWorld/items/ItemSprite.hpp>

template<RE::Renderer> class ItemUser;
template<RE::Renderer> class Inventory;

enum class SlotSelectionManner {
    ABS,
    RIGHT,
    LEFT,
    PREV,
    LAST_SLOT
};

/**
 * @brief Renders and manipulates inventories.
*/
template<RE::Renderer R>
class InventoryUI {
public:

    enum Connection { PRIMARY, SECONDARY, TERTIARY, COUNT };

    /**
     * @brief Contructs a UI that is not connected to any inventories
    */
    InventoryUI(RE::SpriteBatch<R>& sb, const glm::vec2& windowSize);
    ~InventoryUI();

    InventoryUI(const InventoryUI<R>&) = delete;
    InventoryUI<R>& operator=(const InventoryUI<R>&) = delete;

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
    void connectToInventory(Inventory<R>* inventory, Connection connection);

    /**
     * @brief Connects the UI with given item user (disconnects the previous)
     * @param itemUser The item user to connect with. nullptr effectively disconnects the previous item user
    */
    void connectToItemUser(ItemUser<R>* itemUser) { m_itemUser = itemUser; }

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
    void draw(const glm::vec2& cursorPx);

private:

    static constexpr glm::vec2 SLOT_PADDING = glm::vec2(8.0f, 8.0f);

    inline glm::vec2 slotDims() const { return m_slotTex.getSubimageDims(); }
    inline glm::vec2 slotPivot() const { return m_slotTex.getPivot(); }

    inline glm::ivec2 invSize(Connection con) const;
    inline int invSlotCount(Connection con) const;

    std::optional<glm::ivec2> cursorToSlot(const glm::vec2& cursorPx) const;

    template<typename Func>
    void forEachConnectedInventory(Func f) {
        for (int c = PRIMARY; c < static_cast<int>(Connection::COUNT); ++c) {
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

    RE::SpriteBatch<R>& m_sb;
    ItemUser<R>* m_itemUser = nullptr;

    glm::vec2 m_windowSize;
    RE::Texture<R> m_slotTex{{.file = "slot"}};
    glm::vec2 m_invBotLeftPx; /**< Bottom left corner of slot (0, 0) */

    Item m_heldItem{};
    ItemSprite<R> m_heldSprite{};

    int m_chosenSlot = 0;//Is signed but never should be negative
    int m_chosenSlotPrev = 0;//Is signed but never should be negative

    Inventory<R>* m_inv[static_cast<size_t>(Connection::COUNT)] = {nullptr, nullptr, nullptr};
    std::vector<ItemSprite<R>> m_invItemSprites[static_cast<size_t>(Connection::COUNT)];
    bool m_open = false;
};