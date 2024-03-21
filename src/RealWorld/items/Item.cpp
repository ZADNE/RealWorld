/*!
 *  @author    Dubsky Tomas
 */
#include <algorithm>

#include <glm/common.hpp>

#include <RealWorld/items/Item.hpp>

namespace rw {

Item::Item(const ItemSample& sample, int amount)
    : ItemSample(sample)
    , amount(amount) {
}

Item::Item(ItemId id, int amount, float special /* = 0.0f*/)
    : ItemSample(id, special)
    , amount(amount) {
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
    int maxStack = ItemDatabase::md(id).maxStack;
    int temp =
        glm::min(maxStack - amount, (int)(glm::ceil((float)item.amount * portion)));
    amount += temp;
    item.amount -= temp;
    if (item.amount <= 0) {
        item.id = ItemId::Empty;
    }
}

void Item::insert(Item& item, float portion) {
    if (id != ItemId::Empty) {
        // This is not empty item, cannot insert
        return;
    }
    special      = item.special;
    id           = item.id;
    int maxStack = ItemDatabase::md(id).maxStack;
    int temp = glm::min(maxStack, (int)(glm::ceil((float)item.amount * portion)));
    amount += temp;
    item.amount -= temp;
    if (item.amount <= 0) {
        item.id = ItemId::Empty;
    }
}

int Item::operator--() {
    int tmp = amount;
    if (--amount <= 0) {
        id = ItemId::Empty;
    }
    return tmp;
}

int Item::operator--(int) {
    if (--amount <= 0) {
        id = ItemId::Empty;
    }
    return amount;
}

int Item::operator+=(int number) {
    amount += number;
    if (amount <= 0) {
        id = ItemId::Empty;
    }
    return amount;
}

int Item::operator-=(int number) {
    amount -= number;
    if (amount <= 0) {
        id = ItemId::Empty;
    }
    return amount;
}

Item Item::operator*(int number) const {
    Item returnItem = *this;
    returnItem.amount *= number;
    return returnItem;
}

} // namespace rw
