#pragma once
#include <memory>
#include <algorithm>

#include <RealWorld/items/IDB.hpp>
#include <RealWorld/items/Item.hpp>

class ItemInstruction;

class InputProxyIterator {
	friend class IIInputProxy;
public:
	void operator++();
	bool operator!=(InputProxyIterator& IPI);
	const Item& operator*();
private:
	InputProxyIterator(uint8_t index, const ItemInstruction& II) : m_index(index), m_II(II) {}
	uint8_t m_index;
	const ItemInstruction& m_II;
};

class OutputProxyIterator {
	friend class IIOutputProxy;
public:
	void operator++();
	bool operator!=(OutputProxyIterator& IPI);
	const Item& operator*();
private:
	OutputProxyIterator(uint8_t index, const ItemInstruction& II) : m_index(index), m_II(II) {}
	uint8_t m_index;
	const ItemInstruction& m_II;
};

class IIInputProxy {
	friend class ItemInstruction;
public:
	InputProxyIterator begin() const;
	InputProxyIterator end() const;
private:
	IIInputProxy(const ItemInstruction& II): m_II(II) {}
	const ItemInstruction& m_II;
};

class IIOutputProxy {
	friend class ItemInstruction;
public:
	OutputProxyIterator begin() const;
	OutputProxyIterator end() const;
private:
	IIOutputProxy(const ItemInstruction& II) : m_II(II) {}
	const ItemInstruction& m_II;
};


class ItemInstruction {
	friend class WorldRoom;
	friend class InputProxyIterator;
	friend class OutputProxyIterator;
public:
	//Empty instruction
	ItemInstruction() {};

	ItemInstruction(uint8_t inputs, uint8_t outputs, uint8_t mainOutputs = 1):
		m_inputs(new Item[inputs]),
		m_outputs(new Item[outputs]),
		m_inputN(inputs),
		m_outputN(outputs),
		m_mainOutputN((std::min)(mainOutputs, outputs)){
	
	};

	bool operator==(const ItemInstruction& other) const;

	uint8_t numberOfInputs() const;
	uint8_t numberOfOutputs() const;
	uint8_t numberOfMainOutputs() const;

	Item input(unsigned int index) const;
	Item output(unsigned int index) const;

	IIInputProxy inputs() const;
	IIOutputProxy outputs() const;

	unsigned int craftTime() const;

private:
	uint8_t m_inputN = 0u;
	uint8_t m_outputN = 0u;
	uint8_t m_mainOutputN = 0u;//mainOutputN <= outputN

	std::shared_ptr<Item[]> m_inputs;
	std::shared_ptr<Item[]> m_outputs;

	unsigned int m_timeNeeded = 1u;
};