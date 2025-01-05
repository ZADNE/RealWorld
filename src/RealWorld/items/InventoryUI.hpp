/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <optional>

#include <glm/vec2.hpp>

#include <RealEngine/graphics/batches/SpriteBatch.hpp>
#include <RealEngine/graphics/fonts/RasterizedFont.hpp>
#include <RealEngine/resources/ResourceIndex.hpp>

#include <RealWorld/items/Item.hpp>
#include <RealWorld/items/ItemSprite.hpp>

namespace rw {

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
    explicit InventoryUI(glm::vec2 windowSize);

    InventoryUI(const InventoryUI&)            = delete; ///< Noncopyable
    InventoryUI& operator=(const InventoryUI&) = delete; ///< Noncopyable

    InventoryUI(InventoryUI&&)             = delete;     ///< Nonmovable
    InventoryUI&& operator=(InventoryUI&&) = delete;     ///< Nonmovable

    ~InventoryUI();

    /**
     * @brief Notifies the UI that the window has been resized
     */
    void windowResized(glm::vec2 newWindowSize);

    /**
     * @brief Connects the UI with given inventory (connection is mutual)
     * @param inventory The inventory to connect with. nullptr effectively
     * disconnects the previous inventory
     * @param connection The type of conenction that is to be established
     *
     * Simultaneously disconnects the previous inventory.
     */
    void connectToInventory(Inventory* inventory, Connection connection);

    /**
     * @brief   Switches the inventory to the oposite state
     * @details May be blocked if an item is held.
     */
    void switchOpenClose();
    bool isOpen() const { return m_open; }

    void reload();

    void swapUnderCursor(glm::vec2 cursorPx);
    void movePortion(glm::vec2 cursorPx, float portion);

    /**
     * @brief Selects a slot from the first row of the inventory.
     *
     * The item in the selected slot is used by connected item user.
     * @param selectionManner Specifies interpretation of number parameter
     * @param number The slot to select (for some manners irrelevant)
     */
    void selectSlot(SlotSelectionManner selectionManner, int number = 0);
    int selectedSlot() const { return m_selSlot; }

    void step();
    void draw(re::SpriteBatch& spriteBatch, glm::vec2 cursorPx);

private:
    constexpr static glm::vec2 k_slotPadding = glm::vec2(8.0f, 8.0f);

    glm::vec2 slotDims() const { return m_slotTex.subimageDims(); }
    glm::vec2 slotPivot() const { return m_slotTex.pivot(); }

    inline glm::ivec2 invSize(Connection c) const;
    inline int invSlotCount(Connection c) const;

    std::optional<glm::ivec2> cursorToSlot(glm::vec2 cursorPx) const;

    void drawSlot(re::SpriteBatch& spriteBatch, glm::vec2 pivotPx, int slotIndex) const;
    void drawItemCount(
        re::SpriteBatch& spriteBatch, glm::vec2 pivotPx, int itemCount
    ) const;

    template<typename Func>
    void forEachConnectedInventory(Func f) {
        for (int c = Primary; c < static_cast<int>(Connection::Count); ++c) {
            if (m_inv[c]) { // If connected the inventory
                f(static_cast<Connection>(c));
            }
        }
    }

    template<typename Func>
    void forEachSlotByIndex(Connection con, Func f) {
        for (int i = 0; i < invSlotCount(con); ++i) { f(i); }
    }

    template<typename Func>
    void forEachSlotByPosition(Connection con, Func f) {
        for (int y = 0; y < invSize(con).y; y++) {
            for (int x = 0; x < invSize(con).x; x++) { f(x, y); }
        }
    }

    glm::vec2 m_windowSize{};
    re::TextureShaped m_slotTex = re::RM::textureUnmanaged(re::textureID("slot"));
    glm::vec2 m_invBotLeftPx{}; ///< Bottom left corner of slot (0, 0)

    Item m_heldItem{};
    ItemSprite m_heldSprite{};

    int m_selSlot     = 0; ///< Signed but never should be negative
    int m_selSlotPrev = 0; ///< signed but never should be negative

    template<typename T>
    using PerConnection = std::array<T, static_cast<size_t>(Connection::Count)>;

    PerConnection<Inventory*> m_inv{nullptr, nullptr, nullptr};
    PerConnection<std::vector<ItemSprite>> m_invItemSprites;
    bool m_open = false;

    re::RasterizedFont m_countFont{{
        .filePath  = "fonts/arial.ttf",
        .pointSize = 26 // NOLINT(*-magic-numbers): Matches slot design
    }};
};

} // namespace rw
