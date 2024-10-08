/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <vector>

#include <RealWorld/items/InventoryData.hpp>
#include <RealWorld/items/InventoryUI.hpp>

namespace rw {

/**
 * @brief Is a clever container for items.
 */
class Inventory: public InventoryData {
    friend class InventoryUI;
    friend class ItemCombinator;

public:
    explicit Inventory(glm::ivec2 size)
        : InventoryData{size} {}

    Inventory(const Inventory&)            = delete; ///< Noncopyable
    Inventory& operator=(const Inventory&) = delete; ///< Noncopyable

    Inventory(Inventory&&)            = delete;      ///< Nonmovable
    Inventory& operator=(Inventory&&) = delete;      ///< Nonmovable

    ~Inventory();

    // New slots are empty
    // Contents of removed slots are lost
    void resize(glm::ivec2 newSize);

    // Inputs item, creating new stacks
    // Returns true if all the intended items have been inserted, false if not
    // (inventory is full)
    bool insert(
        Item& item, float portion = 1.0f,
        glm::ivec2 startSlot = glm::ivec2(0, 0), bool reload = true
    );
    // Inputs item, first trying to fill already existing stack of this item
    // Returns true if all the intended items have been inserted, false if not
    // (inventory is full)
    bool fill(
        Item& item, float portion = 1.0f,
        glm::ivec2 startSlot = glm::ivec2(0, 0), bool reload = true
    );
    // Item describes what item and how much of it should be removed
    // Return how much MORE was supposed to be yet removed => returning 0 means
    // all items were removed as it was supposed
    int remove(
        const Item& item, glm::ivec2 startSlot = glm::ivec2(0, 0), bool reload = true
    );

    void adoptInventoryData(const InventoryData& id);

    void gatherInventoryData(InventoryData& id) const;

    // fill(item, 1.0f, glm::ivec2(0, 0));
    void operator+=(Item& item);
    // remove(item, glm::ivec2(0, 0));
    int operator-=(const Item& item);

    InventoryData::Proxy operator[](int x) {
        return InventoryData::Proxy(*this, x);
    }

    /**
     * @brief Notifies connected objects that they should reload
     */
    void wasChanged() const;

private:

    // Should only be called from InventoryUI::connectToInventory()
    void connectToUI(InventoryUI* ui, InventoryUI::Connection connection);

    InventoryUI* m_ui = nullptr;
    InventoryUI::Connection m_uiConnection{}; ///< The type of connection to UI
};

} // namespace rw
