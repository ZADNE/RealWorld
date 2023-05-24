/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/Item.hpp>

#include <algorithm>

#include <glm/common.hpp>

Item::Item(const ItemSample& sample, int amount):
    ItemSample(sample),
    amount(amount){

}

Item::Item(ItemId id, int amount, float special/* = 0.0f*/):
    ItemSample(id, special),
    amount(amount){

}

void Item::merge(Item& item, float portion){
    if (id != item.id) { return; }//Both items are not same type, cannot merge
    if (special != item.special) { return; }//Both items do nat have same special, cannot merge
    int maxStack = ItemDatabase::md(id).maxStack;
    int temp = glm::min(maxStack - amount, (int)(glm::ceil((float)item.amount * portion)));
    amount += temp;
    item.amount -= temp;
    if (item.amount <= 0) {
        item.id = ItemId::Empty;
    }
}

void Item::insert(Item & item, float portion){
    if (id != ItemId::Empty) { return; }//This is not empty item, cannot insert
    special = item.special;
    id = item.id;
    int maxStack = ItemDatabase::md(id).maxStack;
    int temp = glm::min(maxStack, (int)(glm::ceil((float)item.amount * portion)));
    amount += temp;
    item.amount -= temp;
    if (item.amount <= 0) {
        item.id = ItemId::Empty;
    }
}

void Item::swap(Item& item){
    Item temp{ item };
    item = (*this);
    (*this) = temp;
}

int Item::operator--(){
    int tmp = amount;
    if (--amount <= 0) {
        id = ItemId::Empty;
    }
    return tmp;
}

int Item::operator--(int){
    if (--amount <= 0) {
        id = ItemId::Empty;
    }
    return amount;
}

int Item::operator+=(int number){
    amount += number;
    if (amount <= 0) {
        id = ItemId::Empty;
    }
    return amount;
}

int Item::operator-=(int number){
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