/*!
 *  @author    Dubsky Tomas
 */
#include <algorithm>

#include <glm/common.hpp>

#include <RealWorld/items/Item.hpp>

namespace rw {

Item::Item(const ItemSample& sample, int count)
    : ItemSample(sample)
    , count(count) {
}

Item::Item(ItemId id, int count, float special /* = 0.0f*/)
    : ItemSample(id, special)
    , count(count) {
}

void Item::merge(Item& item, float portion) {
    if (id != item.id) {
        // Items are not of same type, cannot merge
        return;
    }
    if (special != item.special) {
        // Items do not have the same special, cannot merge
        return;
    }
    int temp = glm::min(
        maxStack(id) - count, (int)(glm::ceil((float)item.count * portion))
    );
    count += temp;
    item.count -= temp;
    if (item.count <= 0) {
        item.id = ItemId::Empty;
    }
}

void Item::insert(Item& item, float portion) {
    if (id != ItemId::Empty) {
        // This is not empty item, cannot insert
        return;
    }
    special = item.special;
    id      = item.id;
    int temp = glm::min(maxStack(id), (int)(glm::ceil((float)item.count * portion)));
    count += temp;
    item.count -= temp;
    if (item.count <= 0) {
        item.id = ItemId::Empty;
    }
}

int Item::operator--() {
    int tmp = count;
    if (--count <= 0) {
        id = ItemId::Empty;
    }
    return tmp;
}

int Item::operator--(int) {
    if (--count <= 0) {
        id = ItemId::Empty;
    }
    return count;
}

int Item::operator+=(int number) {
    count += number;
    if (count <= 0) {
        id = ItemId::Empty;
    }
    return count;
}

int Item::operator-=(int number) {
    count -= number;
    if (count <= 0) {
        id = ItemId::Empty;
    }
    return count;
}

Item Item::operator*(int number) const {
    Item returnItem = *this;
    returnItem.count *= number;
    return returnItem;
}

} // namespace rw
