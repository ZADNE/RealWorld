/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemSample.hpp>

bool ItemSample::isEmpty() const {
    if (id == ItemId::Empty) {
        return true;
    }
    return false;
}