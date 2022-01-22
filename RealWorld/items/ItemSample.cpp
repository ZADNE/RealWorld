#include <RealWorld/items/ItemSample.hpp>

bool ItemSample::operator==(const ItemSample& other) const {
	if (ID != other.ID || special != special) {
		return false;
	}
	return true;
}

bool ItemSample::isEmpty() const {
	if (ID == I_ID::EMPTY) {
		return true;
	}
	return false;
}