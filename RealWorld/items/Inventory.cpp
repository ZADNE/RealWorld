#include <RealWorld/items/Inventory.hpp>

#include <fstream>

#include <RealEngine/Error.hpp>

#include <RealWorld/items/InventoryDrawer.hpp>
#include <RealWorld/items/ItemCombinator.hpp>


Inventory::Inventory(const glm::ivec2& size){
	m_data.size = size;
	//Creating row
	std::vector<Item> row;
	row.resize(size.y);
	//Inserting rows
	for (int x = 0; x < size.x; x++) {
		m_data.items.push_back(row);
	}
}

Inventory::~Inventory(){
	//Disconnecting connected objects (to avoid dangling pointers)
	if (m_inventoryDrawer) {
		m_inventoryDrawer->connectToInventory(nullptr, m_invDrawerConnection);
	}
	if (m_itemCombinator) {
		m_itemCombinator->connectToInventory(nullptr);
	}
}

void Inventory::resize(const glm::ivec2& newSize){
	m_data.resize(newSize);

	//Notifying connected objects about the change
	if (m_inventoryDrawer) {//If connected to any
		m_inventoryDrawer->reloadEverything();
	}
	if (m_itemCombinator) {//If connected to any
		m_itemCombinator->reload();
	}
}

void Inventory::connectToDrawer(InventoryDrawer* inventoryDrawer, Connection connection){
	m_inventoryDrawer = inventoryDrawer;
	m_invDrawerConnection = connection;
}

void Inventory::connectToItemCombinator(ItemCombinator* itemCombinator){
	m_itemCombinator = itemCombinator;
}

bool Inventory::insert(Item& item, float portion/* = 1.0f*/, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/){
	if (item.amount <= 0) { return true; }
	int target = item.amount - (int)(std::ceil((float)item.amount * portion));
	for (int y = startSlot.y; y < m_data.size.y; y++) {
		for (int x = startSlot.x; x < m_data.size.x; x++) {
			m_data.items[x][y].insert(item, portion);
			if (item.amount == target) {
				if (reload) { wasChanged(); }
				return true;
			}
			m_data.items[x][y].merge(item, portion);
			if (item.amount == target) {
				if (reload) { wasChanged(); }
				return true;
			}
		}
	}
	if (reload) { wasChanged(); }//Did not input all the items but might have input some
	return false;
}

bool Inventory::fill(Item& item, float portion/* = 1.0f*/, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/){
	if (item.amount <= 0) { return true; }
	int target = item.amount - (int)((float)item.amount * portion);
	//Filling already existing stacks
	for (int y = startSlot.y; y < m_data.size.y; y++) {
		for (int x = startSlot.x; x < m_data.size.x; x++) {
			m_data.items[x][y].merge(item, portion);
			if (item.amount == target) {
				if (reload) { wasChanged(); }
				return true;
			}
		}
	}
	//Creating new stacks
	for (int y = startSlot.y; y < m_data.size.y; y++) {
		for (int x = startSlot.x; x < m_data.size.x; x++) {
			m_data.items[x][y].insert(item, portion);
			if (item.amount == target) {
				if (reload) { wasChanged(); }
				return true;
			}
		}
	}
	if (reload) { wasChanged(); }//Did not input all the items but might have input some
	return false;
}

int Inventory::remove(const Item& item, const glm::ivec2& startSlot/* = glm::ivec2(0, 0)*/, bool reload/* = true*/){
	if (item.amount <= 0) { return 0; }
	int leftToRemove = item.amount;
	for (int y = startSlot.y; y < m_data.size.y; y++) {
		for (int x = startSlot.x; x < m_data.size.x; x++) {
			Item& slotItem = m_data.items[x][y];
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
	if (m_inventoryDrawer) {//If connected to any
		m_inventoryDrawer->reloadEverything();
	}
	if (m_itemCombinator) {//If connected to any
		m_itemCombinator->reload();
	}
}

void Inventory::adoptInventoryData(const InventoryData& id){
	m_data = id;
	wasChanged();
}

void Inventory::gatherInventoryData(InventoryData& id){
	id = m_data;
	wasChanged();
}

void Inventory::operator+=(Item& item){
	fill(item, 1.0f);
}

int Inventory::operator-=(const Item& item){
	return remove(item);
}
