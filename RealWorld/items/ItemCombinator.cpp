#include <RealWorld/items/ItemCombinator.hpp>

#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemInstructionDatabase.hpp>
#include <RealWorld/items/CraftingDrawer.hpp>
#include <RealWorld/items/ItemInstructionDatabase.hpp>


ItemCombinator::ItemCombinator(){

}

ItemCombinator::~ItemCombinator(){
	if (m_inventory) {
		m_inventory->connectToItemCombinator(nullptr);
	}
	if (m_craftingDrawer) {
		m_craftingDrawer->connectToItemCombinator(nullptr);
	}
	if (m_database) {
		m_database->connectToItemCombinator(nullptr);
	}
}

bool ItemCombinator::craft(const ItemInstruction* itemI, int times, bool testMode){
	if (times <= 0) { return false; }//Cannot craft negative or zero times
	if (!m_database) { return false; }//Cannot craft if not having any instruction database
	if (!itemI) { return false; }//Instruction is nullptr
	std::unordered_map <ItemSample, int, ItemSampleHasher> invMap = m_invMap;
	std::vector<Item> itemsToBeRemoved;
	bool returnVal = craftInternal(itemI, times, testMode, invMap, itemsToBeRemoved);
	if (returnVal && !testMode) {//If crafting was successful and we are NOT just testing, invMap defines current state of the inventory
		m_invMap = invMap;
		m_queue.emplace_back(nullptr, 0);//Border between orders
		//Remove materials from actual inventory
		for (auto& item : itemsToBeRemoved) {
			m_inventory->remove(item, glm::ivec2(0, 0), false);
		}
		m_inventory->wasChanged();
	}
	return returnVal;
}

int ItemCombinator::canCraft(const ItemInstruction* itemI){
	if (!m_database) { return CRAFT_ERROR_NO_DATABASE; }//Cannot craft if not having any instruction database
	int returnVal = 0;
	return returnVal;
}

void ItemCombinator::getCraftableInstructions(std::vector<std::pair<const ItemInstruction*, int>>& craftableInstructions){
	craftableInstructions.clear();
	//Creating set of all instructions that can be crafted at least once
	for (size_t i = 0u; i < m_database->getNumberOfInstructions(); ++i) {
		int canBeCrafted = craft(m_database->getInstruction(i), true, 1);
		if (canBeCrafted > 0) {//If this instruction can be crafted at least once
			craftableInstructions.emplace_back(m_database->getInstruction(i), canBeCrafted);
		}
	}
}

CancelError ItemCombinator::cancel(size_t order, size_t instrOfTheOrder){
	if (m_queue.empty()) { return CancelError::QueueEmpty; }
	if (order == 0u && instrOfTheOrder == 0u && m_progress > 0.0f) {
		return CancelError::InstructionInProgress;
	}
	std::unordered_map <ItemSample, int, ItemSampleHasher> itemsInQueue;
	//Now we need to advance to the order
	size_t currentOrder = 0u;
	auto it = m_queue.begin();
	if (order == 0u) {//Cancelling inside first order
		for (size_t i = 0u; i < instrOfTheOrder; ++i) {
			if (!(it->first)) {
				return CancelError::InstructionTooHigh;//We found padding
			}
			//We need to add outputs of this instruction to items that will be crafted
			for (auto& out : it->first->outputs()) {
				addItemToMap(out * it->second, itemsInQueue);
			}
			++it;
		}
		//it finally points to wanted instruction, now we cancel all orders until we find next padding
		do {
			//First we need to return inputs of the instruction (if they are not in queue)
			for (auto& in : it->first->inputs()) {
				Item item = Item{ in, in.amount * it->second - getAmountOfItemInMap(in, itemsInQueue) };
				m_inventory->fill(item);
				removeItemFromMap(item, itemsInQueue);
			}
			it = m_queue.erase(it);
		} while (it->first);
		return CancelError::Success;
	}

	while ((++it) != m_queue.end()) {
		if (!it->first && ++currentOrder == order) {//If it is padding before the wanted order
			++it;//it is now pointing to the first instruction of the wanted order
			for (size_t i = 0u; i < instrOfTheOrder; ++i) {
				if (!(it->first)) {
					return CancelError::InstructionTooHigh;//We found padding
				}
				//We need to add outputs of this instruction to items that will be crafted
				for (auto& out : it->first->outputs()) {
					addItemToMap(out * it->second, itemsInQueue);
				}
				++it;
			}
			//it finally points to wanted instruction, now we cancel all orders until we find next padding
			do {
				//We need to add outputs of this instruction to items that would be crafted
				for (uchar out = 0u; out < it->first->numberOfOutputs(); ++out) {
					addItemToMap(it->first->output(out) * it->second, itemsInQueue);
				}
				//Second we need to return inputs of the instruction (if they will not be crafted)
				for (auto& in : it->first->inputs()) {
					Item item = Item{ in, in.amount * it->second - getAmountOfItemInMap(in, itemsInQueue) };
					m_inventory->fill(item);
					removeItemFromMap(item, itemsInQueue);
				}
				it = m_queue.erase(it);
			} while (it->first);
			if (instrOfTheOrder == 0u) {
				m_queue.erase(it);//Avoiding double padding
			}
			return CancelError::Success;
		}
	}
	return CancelError::OrderTooHigh;
}

void ItemCombinator::reload(){
	//Clearing previous
	m_invMap.clear();
	//Creating new map
	for (int x = 0; x < m_inventory->getSize().x; ++x) {
		for (int y = 0; y < m_inventory->getSize().y; ++y) {
			addItemToMap((*m_inventory)[x][y], m_invMap);
		}
	}
	//Connected crafting drawer needs to reload too
	if (m_craftingDrawer) {
		m_craftingDrawer->reloadEverything();
	}
}

void ItemCombinator::step(){
	if (m_queue.empty()) {
		return;
	}
	//Crafting something
	m_progress += m_craftingSpeed;
	std::pair<const ItemInstruction*, int> crafted = m_queue.front();
	if ((ushort)m_progress >= (crafted.first->craftTime() * crafted.second)) {//If instruction finished
		std::pair<const ItemInstruction*, int> finished = m_queue.front();
		m_queue.pop_front();
		if (!m_queue.front().first) {//If at the end of order (outputing final product)
			for (auto& output : crafted.first->outputs()) {
				Item item = output * crafted.second;
				m_inventory->fill(item, 1.0f, glm::ivec2(0, 0), false);
				//FULL INVENTORY!!! FUTURE BUG!!!
				//FULL INVENTORY!!! FUTURE BUG!!!
				//FULL INVENTORY!!! FUTURE BUG!!!
			}
			m_inventory->wasChanged();
			m_queue.pop_front();//Popping the border
		}
		m_progress = 0.0f;
		m_craftingDrawer->instructionFinished();
	}
}

void ItemCombinator::connectToIID(ItemInstructionDatabase* database){
	if (m_database) {
		m_database->connectToItemCombinator(nullptr);
	}
	m_database = database;
	if (m_database) {
		m_database->connectToItemCombinator(this);
	}
}

bool ItemCombinator::craftInternal(const ItemInstruction * itemI, int times, bool testMode, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap, std::vector<Item>& itemsToBeRemoved){
	//Check if all inputs are available
	uchar currInput = 0u;
	for (auto& input : itemI->inputs()) {
		++currInput;
		ItemSample sample = ItemSample{input};
		auto inputItem = invMap.find(sample);
		int amountOfInput = getAmountOfItemInMap(sample, invMap);
		int amountOfInputNeeded = input.amount * times;
		//First we take what we have inside inventory
		removeItemFromMap(Item{sample, amountOfInputNeeded}, invMap);
		amountOfInputNeeded -= amountOfInput;

		if (amountOfInputNeeded > 0) {//If still not having enough of the input
			std::vector<const ItemInstruction*> possibleInstructions;
			m_database->whatOutputs(sample, possibleInstructions);
			if (possibleInstructions.size() == 0u) {//If cannot craft this item (and do not have it)
				//First we need to return items to the inventory map
				for (uchar i = 0u; i < (currInput - 1u); ++i) {
					addItemToMap(Item{ sample, input.amount * times }, invMap);
				}
				addItemToMap(Item{ sample, input.amount * times }, invMap);
				return false;
			}
			//We have instructions to craft it, lets try if we have enough materials to craft any
			bool wasCrafted = false;
			for (auto& pi : possibleInstructions) {
				//Item can be crafted from more output slots
				for (auto& output : pi->outputs()) {
					int timesToCraft = (amountOfInputNeeded + output.amount - 1) / output.amount;//int ceil
					if (craftInternal(pi, timesToCraft, testMode, invMap, itemsToBeRemoved)) {//Found instruction that crafts all the items
						wasCrafted = true;
						break;
					}
					if (wasCrafted) { break; }
				}
			}
			if (!wasCrafted) {//We do not have enough materials to finish any of possible instructions, cannot craft this item
				//First we need to return items to the inventory map
				for (uchar i = 0u; i < currInput; ++i) {
					addItemToMap(Item{ sample, input.amount * times }, invMap);
				}
				return false;
			}
		}
	}
	//All inputs are available
	if (!testMode) {//If not in the test mode
		//We can add this instruction to the queue
		m_queue.emplace_back(itemI, times);
		//Add inputs into to-be-removed vector
		for (auto& input : itemI->inputs()) {
			itemsToBeRemoved.emplace_back(input * times);
		}
	}
	return true;
}

void ItemCombinator::addItemToMap(const Item& item, std::unordered_map <ItemSample, int, ItemSampleHasher>& invMap){
	if (!item.isEmpty()) {//If the item is not empty
		auto it = invMap.find(ItemSample(item));
		if (it != invMap.end()) {//If this entry is already in the map
			it->second += item.amount;
		}
		else {//Creating new entry
			invMap.insert(std::make_pair(ItemSample(item), item.amount));
		}
	}
}

int ItemCombinator::getAmountOfItemInMap(const Item& item, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap) const {
	return getAmountOfItemInMap(ItemSample(item), invMap);
}

int ItemCombinator::getAmountOfItemInMap(const ItemSample& itemSample, std::unordered_map<ItemSample, int, ItemSampleHasher>& invMap) const{
	if (itemSample.isEmpty()) { return 0; }
	auto it = invMap.find(itemSample);
	if (it != invMap.end()) {//If this entry is already in the map
		return it->second;
	}
	return 0;
}

void ItemCombinator::removeItemFromMap(const Item& item, std::unordered_map <ItemSample, int, ItemSampleHasher>& invMap){
	if (!item.isEmpty()) {//If the item is not empty
		auto it = invMap.find(ItemSample(item));
		if (it != invMap.end()) {//If this entry is already in the map
			it->second -= item.amount;
			if (it->second <= 0) {//If there are no more 
				invMap.erase(it);
			}
		}
	}
}

void ItemCombinator::connectToInventory(Inventory* inventory){
	//Disconnecting from previous inventory
	if (m_inventory) {//If already connected to one
		m_inventory->connectToItemCombinator(nullptr);
	}
	m_inventory = inventory;
	//Clearing map
	m_invMap.clear();

	//TODO: INSTRUCTIONS IN PROGRESS!!!

	if (m_inventory) {//If connecting new inventory
		//Notifying new inventory
		m_inventory->connectToItemCombinator(this);
		//Creating new map
		reload();
	}
}

void ItemCombinator::connectToCraftingDrawer(CraftingDrawer* craftingDrawer){
	m_craftingDrawer = craftingDrawer;
}