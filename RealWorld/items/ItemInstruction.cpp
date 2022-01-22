#include <RealWorld/items/ItemInstruction.hpp>

bool ItemInstruction::operator==(const ItemInstruction& other) const{
	if (m_inputN != other.numberOfInputs() || m_outputN != other.numberOfOutputs() || m_timeNeeded != other.craftTime()) {
		return false;
	}
	//Check all inputs whether they are same
	for (unsigned int i = 0u; i < m_inputN; i++) {
		if (m_inputs[i] != other.input(i)) {
			return false;
		}
	}
	//Check all outputs whether they are same
	for (unsigned int i = 0u; i < m_outputN; i++) {
		if (m_outputs[i] != other.output(i)) {
			return false;
		}
	}
	return true;
}

uint8_t ItemInstruction::numberOfInputs() const{
	return m_inputN;
}

uint8_t ItemInstruction::numberOfOutputs() const{
	return m_outputN;
}

uint8_t ItemInstruction::numberOfMainOutputs() const{
	return m_mainOutputN;
}

Item ItemInstruction::input(unsigned int index) const{
	if (index < m_inputN) {
		return m_inputs[index];
	}
	//Out of range, returning empty item
	return Item{};
}

Item ItemInstruction::output(unsigned int index) const{
	if (index < m_outputN) {
		return m_outputs[index];
	}
	//Out of range, returning empty item
	return Item{};

}

IIInputProxy ItemInstruction::inputs() const{
	return IIInputProxy(*this);
}

IIOutputProxy ItemInstruction::outputs() const{
	return IIOutputProxy(*this);
}

unsigned int ItemInstruction::craftTime() const{
	return m_timeNeeded;
}

InputProxyIterator IIInputProxy::begin() const{
	return InputProxyIterator{0u, m_II};
}

InputProxyIterator IIInputProxy::end() const{
	return InputProxyIterator{m_II.numberOfInputs(), m_II };
}

void InputProxyIterator::operator++(){
	++m_index;
}

bool InputProxyIterator::operator!=(InputProxyIterator& IPI){
	return (m_index != IPI.m_index) ? true : false;
}

const Item& InputProxyIterator::operator*(){
	return m_II.m_inputs[m_index];
}

void OutputProxyIterator::operator++(){
	++m_index;
}

bool OutputProxyIterator::operator!=(OutputProxyIterator& IPI){
	return (m_index != IPI.m_index) ? true : false;
}

const Item& OutputProxyIterator::operator*(){
	return m_II.m_outputs[m_index];
}

OutputProxyIterator IIOutputProxy::begin() const{
	return OutputProxyIterator{0u, m_II};
}

OutputProxyIterator IIOutputProxy::end() const{
	return OutputProxyIterator{ m_II.numberOfOutputs(), m_II };
}
