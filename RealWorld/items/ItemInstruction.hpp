#pragma once
#include <memory>
#include <algorithm>
//#include <array>

#include "IDB.hpp"
#include "Item.hpp"

class ItemInstruction;

class InputProxyIterator {
	friend class IIInputProxy;
public:
	void operator++();
	bool operator!=(InputProxyIterator& IPI);
	const Item& operator*();
private:
	InputProxyIterator(uchar index, const ItemInstruction& II) : m_index(index), m_II(II) {}
	uchar m_index;
	const ItemInstruction& m_II;
};

class OutputProxyIterator {
	friend class IIOutputProxy;
public:
	void operator++();
	bool operator!=(OutputProxyIterator& IPI);
	const Item& operator*();
private:
	OutputProxyIterator(uchar index, const ItemInstruction& II) : m_index(index), m_II(II) {}
	uchar m_index;
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

	ItemInstruction(uchar inputs, uchar outputs, uchar mainOutputs = 1):
		m_inputs(new Item[inputs]),
		m_outputs(new Item[outputs]),
		m_inputN(inputs),
		m_outputN(outputs),
		m_mainOutputN((std::min)(mainOutputs, outputs)){
	
	};

	bool operator==(const ItemInstruction& other) const;

	uchar numberOfInputs() const;
	uchar numberOfOutputs() const;
	uchar numberOfMainOutputs() const;

	Item input(uchar index) const;
	Item output(uchar index) const;

	IIInputProxy inputs() const;
	IIOutputProxy outputs() const;

	ushort craftTime() const;

private:
	uchar m_inputN = 0u;
	uchar m_outputN = 0u;
	uchar m_mainOutputN = 0u;//mainOutputN <= outputN

	std::shared_ptr<Item[]> m_inputs;
	std::shared_ptr<Item[]> m_outputs;

	ushort m_timeNeeded = 1u;
};