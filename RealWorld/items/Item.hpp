#pragma once
#include "ItemSample.hpp"

struct Item : public ItemSample {
		//Construct empty item
	Item();
		//Construct item from ItemSample and amount
	Item(const ItemSample& sample, int amount);
		//Construct item from parameters
	Item(I_ID ID, int amount, float special = 0.0f);
	~Item();

		//Merge with another item of same ID and special (checks are done inside this function)
		//Both items may be changed
		//Action is done only with partion of item
	void merge(Item& item, float portion);
		//Insert another item into this item if this item is empty (checks are done inside this function)
		//Action is done only with partion of item
	void insert(Item& item, float portion);
		//Swaps contents of two items
	void swap(Item& item);

		//Operates on amount
	int operator--();
		//Operates on amount
	int operator--(int);
		//Operates on amount
	int operator+=(int number);
		//Operates on amount
	int operator-=(int number);
		//Operates on amount
	Item operator*(int number) const;


	int amount = 0;
};