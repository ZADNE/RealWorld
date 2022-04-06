#include <RealWorld/items/Inventory.hpp>

#include <fstream>

#include <RealEngine/main/Error.hpp>

#include <RealWorld/items/ItemCombinator.hpp>


Inventory::Inventory(const glm::ivec2& size) : InventoryData{size} {

}

Inventory::~Inventory() {
	//Disconnecting connected objects (to avoid dangling pointers)
	if (m_UI) {
		m_UI->connectToInventory(nullptr, m_UIConnection);
	}
	if (m_itemCombinator) {
		m_itemCombinator->connectToInventory(nullptr);
	}
}

void Inventory::resize(const glm::ivec2& newSize) {
	InventoryData::resize(newSize);

	//Notifying connected objects about the change
	if (m_UI) {//If connected to any
		m_UI->reload();
	}
	if (m_itemCombinator) {//If connected to any
		m_itemCombinator->reload();
	}
}

void Inventory::connectToDrawer(InventoryUI* inventoryDrawer, InventoryUI::Connection connection) {
	m_UI = inventoryDrawer;
	m_UIConnection = connection;
}

void Inventory::connectToItemCombinator(ItemCombinator* itemCombinator) {
	m_itemCombinator = itemCombinator;
}

bool Inventory::insert(Item& item, float portion/* = 1.0f*/, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/) {
	if (item.amount <= 0) { return true; }
	int target = item.amount - (int)(std::ceil((float)item.amount * portion));
	for (int y = startSlot.y; y < p_size.y; y++) {
		for (int x = startSlot.x; x < p_size.x; x++) {
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

bool Inventory::fill(Item& item, float portion/* = 1.0f*/, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/) {
	if (item.amount <= 0) { return true; }
	int target = item.amount - (int)((float)item.amount * portion);
	//Filling already existing stacks
	for (int y = startSlot.y; y < p_size.y; y++) {
		for (int x = startSlot.x; x < p_size.x; x++) {
			(*this)[x][y].merge(item, portion);
			if (item.amount == target) {
				if (reload) { wasChanged(); }
				return true;
			}
		}
	}
	//Creating new stacks
	for (int y = startSlot.y; y < p_size.y; y++) {
		for (int x = startSlot.x; x < p_size.x; x++) {
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

int Inventory::remove(const Item& item, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/) {
	if (item.amount <= 0) { return 0; }
	int leftToRemove = item.amount;
	for (int y = startSlot.y; y < p_size.y; y++) {
		for (int x = startSlot.x; x < p_size.x; x++) {
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

void Inventory::wasChanged() const {
	if (m_UI) {//If connected to any
		m_UI->reload();
	}
	if (m_itemCombinator) {//If connected to any
		m_itemCombinator->reload();
	}
}

void Inventory::adoptInventoryData(const InventoryData& id) {
	p_size = id.p_size;
	p_items = id.p_items;
	wasChanged();
}

void Inventory::gatherInventoryData(InventoryData& id) const {
	id.p_size = p_size;
	id.p_items = p_items;
}

void Inventory::operator+=(Item& item) {
	fill(item, 1.0f);
}

int Inventory::operator-=(const Item& item) {
	return remove(item);
}
