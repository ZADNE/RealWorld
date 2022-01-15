#pragma once
#include <unordered_map>
#include <list>
#include <vector>

#include "ItemInstruction.hpp"
#include "Item.hpp"
#include "ItemSample.hpp"

class Inventory;
class CraftingDrawer;
class ItemInstructionDatabase;

const int CRAFT_ERROR_NO_DATABASE = -1;

enum class CancelError{
	Success,
	QueueEmpty,
	InstructionInProgress,
	OrderTooHigh,
	InstructionTooHigh,
};

class ItemCombinator {
	friend class CraftingDrawer;
public:
	ItemCombinator();
	~ItemCombinator();

		//Disconnects from previous and connects to new inventory
		//Passing nullptr effectively disconnects from any inventory
	void connectToInventory(Inventory* inventory);
		//Returns if this instruction could be added to the queue
		//testMode == true -> nothing is added to the queue, nothing is removed from the inventory (for testing whether something can be crafted)
	bool craft(const ItemInstruction* itemI, int times, bool testMode);
		//Returns how many times this instruction can be crafted
		//Negative numbers indicate the reason why it cannot be crafted nor once (it is once of the CRAFT_ERRORs)
	int canCraft(const ItemInstruction* itemI);
		//int indicates how many times it can be crafted
	void getCraftableInstructions(std::vector<std::pair<const ItemInstruction*, int>>& craftableInstructions);

	CancelError cancel(size_t order, size_t instrOfTheOrder);

	void reload();
		//Disconnects from previous and connects to new ItemInstructionDatabase
		//Passing nullptr effectively disconnects from any ItemInstructionDatabase
	void connectToIID(ItemInstructionDatabase* database);

	void step();
private:
		//Disconnects from previous and connects to new crafting drawer
		//Passing nullptr effectively disconnects from any crafting drawer
	void connectToCraftingDrawer(CraftingDrawer* craftingDrawer);
		//Returns if this instruction could be added to the queue
	bool craftInternal(const ItemInstruction* itemI, int times, bool testMode, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap, std::vector<Item>& itemsToBeRemoved);

	void addItemToMap(const Item& item, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap);
	int getAmountOfItemInMap(const Item& item, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap) const;
	int getAmountOfItemInMap(const ItemSample& itemSample, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap) const;
	void removeItemFromMap(const Item& item, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap);

	Inventory* m_inventory = nullptr;
	CraftingDrawer* m_craftingDrawer = nullptr;
	ItemInstructionDatabase* m_database = nullptr;

	std::unordered_map <ItemSample, int, ItemSampleHasher> m_invMap;//Inventory "converted" to map
	std::list<std::pair<const ItemInstruction*, int>> m_queue;//Queue of instructions, int defines how many times the instruction there is in row, nullptr defines border between user started orders
	float m_progress = 0.0f;//Progress in crafting of the first instruction in the queue
	float m_craftingSpeed = 1.0f;//1.0f is the standard speed
};