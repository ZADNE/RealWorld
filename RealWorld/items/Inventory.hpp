#pragma once
#include <vector>

#include <glm\glm.hpp>

#include "InventoryData.hpp"
#include "../metadata.hpp"

enum Connection;
class InventoryDrawer;
class ItemCombinator;

class Inventory {
	friend class InventoryDrawer;
	friend class ItemCombinator;
public:
	Inventory(const glm::ivec2& size);
	~Inventory();
		//New slots are empty
		//Contents of removed slots are lost
	void resize(const glm::ivec2& newSize);

	glm::ivec2 getSize() const { return m_data.size; }
		//Inputs item, creating new stacks
		//Returns true if all the intended items have been inserted, false if not (inventory is full)
	bool insert(Item& item, float portion = 1.0f, const glm::ivec2& startSlot = glm::ivec2(0, 0), bool reload = true);
		//Inputs item, first trying to fill already existing stack of this item
		//Returns true if all the intended items have been inserted, false if not (inventory is full)
	bool fill(Item& item, float portion = 1.0f, const glm::ivec2& startSlot = glm::ivec2(0, 0), bool reload = true);
		//Item describes what item and how much of it should be removed
		//Return how much MORE was supposed to be yet removed => returning 0 means all items were removed as it was supposed
	int remove(const Item& item, const glm::ivec2& startSlot = glm::ivec2(0, 0), bool reload = true);
		//Notifies connected objects that they should reload
	void wasChanged() const;
	
	void adoptInventoryData(const InventoryData& id);

	void gatherInventoryData(InventoryData& id);

		//fill(item, 1.0f, glm::ivec2(0, 0));
	void operator+= (Item& item);
		//remove(item, glm::ivec2(0, 0));
	int operator-= (const Item& item);

	class Proxy {
	public:
		Proxy(std::vector<Item>* vector) : vector(vector) { }

		Item& operator[](int y) {
			return (*vector)[y];
		}
	private:
		std::vector<Item>* vector;
	};

	Proxy operator[](int x) {
		return Proxy(&m_data.items[x]);
	}
private:
		//Should only be called from InventoryDrawer::connectToInventory()
	void connectToDrawer(InventoryDrawer* inventoryDrawer, Connection connection);
		//Should only be called from ItemCombinator::connectToInventory()
	void connectToItemCombinator(ItemCombinator* itemCombinator);

	InventoryDrawer* m_inventoryDrawer = nullptr;
	Connection m_invDrawerConnection;//Defines how it is connected to the drawer
	ItemCombinator* m_itemCombinator = nullptr;
	InventoryData m_data;
};