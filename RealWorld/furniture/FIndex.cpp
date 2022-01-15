#include "FIndex.hpp"


FIndex::FIndex(): m_type(F_TYPE::ERROR), m_index(0u) {

}

FIndex::FIndex(BuildError error): m_type(F_TYPE::ERROR), m_index((size_t)error) {

}

FIndex::FIndex(F_TYPE type, size_t index) : m_type(type), m_index(index) {

}


FIndex::~FIndex() {

}

bool FIndex::isError() const {
	if (m_type == F_TYPE::ERROR) {
		return true;
	}
	return false;
}

BuildError FIndex::getError() const {
	if (!isError()) {
		return BuildError::NO_ERROR;//This is NOT actually an error index
	}
	return (BuildError)m_index;
}

F_TYPE FIndex::getType() const {
	return m_type;
}

size_t FIndex::getIndex() const {
	return m_index;
}
