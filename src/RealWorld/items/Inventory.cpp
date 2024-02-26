/*!
 *  @author    Dubsky Tomas
 */
#include <fstream>

#include <RealEngine/utility/Error.hpp>

#include <RealWorld/items/Inventory.hpp>

namespace rw {

Inventory::~Inventory() {
    // Disconnect connected objects (to avoid dangling pointers)
    if (m_ui) {
        m_ui->connectToInventory(nullptr, m_uiConnection);
    }
}

void Inventory::resize(glm::ivec2 newSize) {
    InventoryData::resize(newSize);
    // Notify connected objects about the change
    if (m_ui) { // If connected to any
        m_ui->reload();
    }
}

bool Inventory::insert(
    Item& item, float portion /* = 1.0f*/,
    glm::ivec2 startSlot /* = glm::ivec2(0, 0)*/, bool reload /* = true*/
) {
    if (item.amount <= 0) {
        return true;
    }
    int target = item.amount - (int)(std::ceil((float)item.amount * portion));
    for (int y = startSlot.y; y < m_dims.y; y++) {
        for (int x = startSlot.x; x < m_dims.x; x++) {
            (*this)[x][y].insert(item, portion);
            if (item.amount == target) {
                if (reload) {
                    wasChanged();
                }
                return true;
            }
            (*this)[x][y].merge(item, portion);
            if (item.amount == target) {
                if (reload) {
                    wasChanged();
                }
                return true;
            }
        }
    }
    if (reload) {
        wasChanged();
    } // Did not input all the items but might have input some
    return false;
}

bool Inventory::fill(
    Item& item, float portion /* = 1.0f*/,
    glm::ivec2 startSlot /* = glm::ivec2(0, 0)*/, bool reload /* = true*/
) {
    if (item.amount <= 0) {
        return true;
    }
    int target = item.amount - (int)((float)item.amount * portion);
    // Filling already existing stacks
    for (int y = startSlot.y; y < m_dims.y; y++) {
        for (int x = startSlot.x; x < m_dims.x; x++) {
            (*this)[x][y].merge(item, portion);
            if (item.amount == target) {
                if (reload) {
                    wasChanged();
                }
                return true;
            }
        }
    }
    // Creating new stacks
    for (int y = startSlot.y; y < m_dims.y; y++) {
        for (int x = startSlot.x; x < m_dims.x; x++) {
            (*this)[x][y].insert(item, portion);
            if (item.amount == target) {
                if (reload) {
                    wasChanged();
                }
                return true;
            }
        }
    }
    if (reload) {
        wasChanged();
    } // Did not input all the items but might have input some
    return false;
}

int Inventory::remove(
    const Item& item, glm::ivec2 startSlot /* = glm::ivec2(0, 0)*/,
    bool reload /* = true*/
) {
    if (item.amount <= 0) {
        return 0;
    }
    int leftToRemove = item.amount;
    for (int y = startSlot.y; y < m_dims.y; y++) {
        for (int x = startSlot.x; x < m_dims.x; x++) {
            Item& slotItem = (*this)[x][y];
            if (slotItem == item) { // If this is the desired item
                int removed = std::min(slotItem.amount, item.amount);
                slotItem -= removed;
                leftToRemove -= removed;
                if (leftToRemove == 0) { // If already have all items removed
                    if (reload) {
                        wasChanged();
                    }
                    return leftToRemove;
                }
            }
        }
    }
    if (reload) {
        wasChanged();
    }
    return leftToRemove;
}

void Inventory::adoptInventoryData(const InventoryData& id) {
    m_dims  = id.m_dims;
    m_items = id.m_items;
    wasChanged();
}

void Inventory::gatherInventoryData(InventoryData& id) const {
    id.m_dims  = m_dims;
    id.m_items = m_items;
}

void Inventory::operator+=(Item& item) {
    fill(item, 1.0f);
}

int Inventory::operator-=(const Item& item) {
    return remove(item);
}

void Inventory::wasChanged() const {
    if (m_ui) { // If connected to any
        m_ui->reload();
    }
}

void Inventory::connectToUI(InventoryUI* ui, InventoryUI::Connection connection) {
    m_ui           = ui;
    m_uiConnection = connection;
}

} // namespace rw
