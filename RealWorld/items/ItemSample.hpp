/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/items/ItemDatabase.hpp>

/**
 * @brief Is an instance of a type of item.
*/
struct ItemSample {

    ItemSample(){};
        //Construct entry from parameters
    ItemSample(ITEM ID, float special) : ID(ID), special(special) {};

        
    bool operator==(const ItemSample& other) const;
    bool operator!=(const ItemSample& other) const { return !operator==(other); };

        //ItemSample is considered empty if its ITEM == ITEM::EMPTY
    bool isEmpty() const;

    ITEM ID = ITEM::EMPTY;
    float special = 0.0f;
};

struct ItemSampleHasher {
    size_t operator()(const ItemSample& otherSample) const {
        return ((size_t)otherSample.ID | (size_t)otherSample.special << 16);
    }
};
