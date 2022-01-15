#pragma once
#include "IDB.hpp"

struct ItemSample {
		//Construct empty ItemSample
	ItemSample(){};
		//Construct entry from parameters
	ItemSample(I_ID ID, float special) : ID(ID), special(special) {};

		
	bool operator==(const ItemSample& other) const;
	bool operator!=(const ItemSample& other) const { return !operator==(other); };

		//ItemSample is considered empty if its I_ID == I_ID::EMPTY
	bool isEmpty() const;

	I_ID ID = I_ID::EMPTY;
	float special = 0.0f;
};

struct ItemSampleHasher {
	size_t operator()(const ItemSample& otherSample) const {
		return ((size_t)otherSample.ID | (size_t)otherSample.special << 16);
	}
};
