#include <RealWorld/items/Item.hpp>

#include <algorithm>


Item::Item():
	ItemSample{} {

}

Item::Item(const ItemSample& sample, int amount):
	ItemSample(sample),
	amount(amount){

}

Item::Item(ITEM ID, int amount, float special/* = 0.0f*/):
	ItemSample(ID, special),
	amount(amount){

}

Item::~Item(){

}

void Item::merge(Item& item, float portion){
	if (ID != item.ID) { return; }//Both items are not same type, cannot merge
	if (special != item.special) { return; }//Both items do nat have same special, cannot merge
	int maxStack = ItemDatabase::md(ID).maxStack;
	int temp = std::min(maxStack - amount, (int)(std::ceil((float)item.amount * portion)));
	amount += temp;
	item.amount -= temp;
	if (item.amount <= 0) {
		item.ID = ITEM::EMPTY;
	}
}

void Item::insert(Item & item, float portion){
	if (ID != ITEM::EMPTY) { return; }//This is not empty item, cannot insert
	special = item.special;
	ID = item.ID;
	int maxStack = ItemDatabase::md(ID).maxStack;
	int temp = std::min(maxStack, (int)(std::ceil((float)item.amount * portion)));
	amount += temp;
	item.amount -= temp;
	if (item.amount <= 0) {
		item.ID = ITEM::EMPTY;
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
		ID = ITEM::EMPTY;
	}
	return tmp;
}

int Item::operator--(int){
	if (--amount <= 0) {
		ID = ITEM::EMPTY;
	}
	return amount;
}

int Item::operator+=(int number){
	amount += number;
	if (amount <= 0) {
		ID = ITEM::EMPTY;
	}
	return amount;
}

int Item::operator-=(int number){
	amount -= number;
	if (amount <= 0) {
		ID = ITEM::EMPTY;
	}
	return amount;
}

Item Item::operator*(int number) const {
	Item returnItem = *this;
	returnItem.amount *= number;
	return returnItem;
}