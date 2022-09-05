/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/Inventory.hpp>

#include <fstream>

#include <RealEngine/utility/error.hpp>


template<RE::Renderer R>
Inventory<R>::Inventory(const glm::ivec2& size) :
    InventoryData{size} {

}

template<RE::Renderer R>
Inventory<R>::~Inventory() {
    //Disconnecting connected objects (to avoid dangling pointers)
    if (m_UI) {
        m_UI->connectToInventory(nullptr, m_UIConnection);
    }
}

template<RE::Renderer R>
void Inventory<R>::resize(const glm::ivec2& newSize) {
    InventoryData::resize(newSize);

    //Notifying connected objects about the change
    if (m_UI) {//If connected to any
        m_UI->reload();
    }
}

template<RE::Renderer R>
void Inventory<R>::connectToDrawer(InventoryUI<R>* inventoryDrawer, InventoryUI<R>::Connection connection) {
    m_UI = inventoryDrawer;
    m_UIConnection = connection;
}

template<RE::Renderer R>
bool Inventory<R>::insert(Item& item, float portion/* = 1.0f*/, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/) {
    if (item.amount <= 0) { return true; }
    int target = item.amount - (int)(std::ceil((float)item.amount * portion));
    for (int y = startSlot.y; y < m_size.y; y++) {
        for (int x = startSlot.x; x < m_size.x; x++) {
            (*this)[x][y].insert(item, portion);
            if (item.amount == target) {
                if (reload) { wasChanged(); }
                return true;
            }
            (*this)[x][y].merge(item, portion);
            if (item.amount == target) {
                if (reload) { wasChanged(); }
                return true;
            }
        }
    }
    if (reload) { wasChanged(); }//Did not input all the items but might have input some
    return false;
}

template<RE::Renderer R>
bool Inventory<R>::fill(Item& item, float portion/* = 1.0f*/, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/) {
    if (item.amount <= 0) { return true; }
    int target = item.amount - (int)((float)item.amount * portion);
    //Filling already existing stacks
    for (int y = startSlot.y; y < m_size.y; y++) {
        for (int x = startSlot.x; x < m_size.x; x++) {
            (*this)[x][y].merge(item, portion);
            if (item.amount == target) {
                if (reload) { wasChanged(); }
                return true;
            }
        }
    }
    //Creating new stacks
    for (int y = startSlot.y; y < m_size.y; y++) {
        for (int x = startSlot.x; x < m_size.x; x++) {
            (*this)[x][y].insert(item, portion);
            if (item.amount == target) {
                if (reload) { wasChanged(); }
                return true;
            }
        }
    }
    if (reload) { wasChanged(); }//Did not input all the items but might have input some
    return false;
}

template<RE::Renderer R>
int Inventory<R>::remove(const Item& item, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/) {
    if (item.amount <= 0) { return 0; }
    int leftToRemove = item.amount;
    for (int y = startSlot.y; y < m_size.y; y++) {
        for (int x = startSlot.x; x < m_size.x; x++) {
            Item& slotItem = (*this)[x][y];
            if (slotItem == item) {//If this is the desired item
                int removed = std::min(slotItem.amount, item.amount);
                slotItem -= removed;
                leftToRemove -= removed;
                if (leftToRemove == 0) {//If already have all items removed
                    if (reload) { wasChanged(); }
                    return leftToRemove;
                }
            }
        }
    }
    if (reload) { wasChanged(); }
    return leftToRemove;
}

template<RE::Renderer R>
void Inventory<R>::wasChanged() const {
    if (m_UI) {//If connected to any
        m_UI->reload();
    }
}

template<RE::Renderer R>
void Inventory<R>::adoptInventoryData(const InventoryData& id) {
    m_size = id.m_size;
    m_items = id.m_items;
    wasChanged();
}

template<RE::Renderer R>
void Inventory<R>::gatherInventoryData(InventoryData& id) const {
    id.m_size = m_size;
    id.m_items = m_items;
}

template<RE::Renderer R>
void Inventory<R>::operator+=(Item& item) {
    fill(item, 1.0f);
}

template<RE::Renderer R>
int Inventory<R>::operator-=(const Item& item) {
    return remove(item);
}

template Inventory<RE::RendererGL46>;
