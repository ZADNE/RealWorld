/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemSample.hpp>

namespace rw {

bool ItemSample::isEmpty() const {
    if (id == ItemId::Empty) {
        return true;
    }
    return false;
}

} // namespace rw