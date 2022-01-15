#include "ItemInstructionDatabase.hpp"

#include <stdexcept>

#include "ItemCombinator.hpp"

ItemInstructionDatabase::ItemInstructionDatabase():
	m_instructions(std::make_unique<std::vector<ItemInstruction>>()),
	m_inputsMap(std::make_unique<std::unordered_multimap<ItemSample, size_t, ItemSampleHasher>>()),
	m_outputsMap(std::make_unique<std::unordered_multimap<ItemSample, size_t, ItemSampleHasher>>()){

}

ItemInstructionDatabase::~ItemInstructionDatabase(){
	if (m_itemCombinator) {
		m_itemCombinator->connectToIID(nullptr);
	}
}

void ItemInstructionDatabase::addInstruction(const ItemInstruction& instruction){
	m_instructions->push_back(instruction);
	//Inputs map
	for (uchar input = 0u; input < instruction.numberOfInputs(); ++input) {
		m_inputsMap->emplace(ItemSample{ instruction.input(input) }, m_instructions->size() - 1u);
	}
	//Outputs map
	for (uchar output = 0u; output < instruction.numberOfOutputs(); ++output) {
		m_outputsMap->emplace(ItemSample{ instruction.output(output) }, m_instructions->size() - 1u);
	}
}

void ItemInstructionDatabase::whatOutputs(const ItemSample& sample, std::vector<const ItemInstruction*>& instructions) {
	size_t count = m_outputsMap->count(sample);
	if (count > 0u) {//If this item can actually be crafted
		instructions.reserve(count);
		auto range = m_outputsMap->equal_range(sample);
		//Push-back all instructions
		for (auto i = range.first; i != range.second; ++i) {
			instructions.push_back(&(*m_instructions)[i->second]);
		}
	}
}

void ItemInstructionDatabase::whatInputs(const ItemSample& sample, std::vector<const ItemInstruction*>& instructions){
	size_t count = m_inputsMap->count(sample);
	if (count > 0u) {//If this item can actually be crafted
		instructions.reserve(count);
		auto range = m_inputsMap->equal_range(sample);
		//Push-back all instructions
		for (auto i = range.first; i != range.second; ++i) {
			instructions.push_back(&(*m_instructions)[i->second]);
		}
	}
}

const ItemInstruction* ItemInstructionDatabase::getInstruction(size_t index){
	try {
		return &m_instructions->at(index);
	}
	catch (std::out_of_range) {
		return nullptr;
	}
}

size_t ItemInstructionDatabase::getNumberOfInstructions(){
	return m_instructions->size();
}

void ItemInstructionDatabase::connectToItemCombinator(ItemCombinator* itemCombinator){
	m_itemCombinator = itemCombinator;
}
