#pragma once
#include <unordered_map>
#include <vector>
#include <memory>

#include "ItemInstruction.hpp"
#include "IDB.hpp"
#include "ItemSample.hpp"

class ItemCombinator;

class ItemInstructionDatabase {
	friend class ItemCombinator;
public:
	ItemInstructionDatabase();
	~ItemInstructionDatabase();

		//Adds new instruction to all known instructions
	void addInstruction(const ItemInstruction& instruction);

		//Lists all instructions that PRODUCE ID to the instructions vector
	void whatOutputs(const ItemSample& sample, std::vector<const ItemInstruction*>& instructions);

		//Lists all instructions that CONSUME ID to the instructions vector
	void whatInputs(const ItemSample& sample, std::vector<const ItemInstruction*>& instructions);

		//index >= number of instructions results in nullptr
	const ItemInstruction* getInstruction(size_t index);
	size_t getNumberOfInstructions();
private:
	void connectToItemCombinator(ItemCombinator* itemCombinator);

	std::unique_ptr<std::vector<ItemInstruction>> m_instructions;//Hard data, vector of all known ItemInstructions

	std::unique_ptr<std::unordered_multimap<ItemSample, size_t, ItemSampleHasher>> m_outputsMap;//For quick look up of item instructions that OUTPUTS ID
	std::unique_ptr<std::unordered_multimap<ItemSample, size_t, ItemSampleHasher>> m_inputsMap;//For quick look up of item instructions that INPUTS ID

	ItemCombinator* m_itemCombinator = nullptr;
};
