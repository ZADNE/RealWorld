/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemSample.hpp>

bool ItemSample::operator==(const ItemSample& other) const {
	if (ID != other.ID || special != special) {
		return false;
	}
	return true;
}

bool ItemSample::isEmpty() const {
	if (ID == ITEM::EMPTY) {
		return true;
	}
	return false;
}