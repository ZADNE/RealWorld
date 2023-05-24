/*! 
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/items/ItemSample.hpp>

/**
 * @brief Representes a number of item samples.
*/
struct Item : public ItemSample {

    /**
     * @brief Constructs empty item
    */
    Item() {};

    /**
     * @brief Constructs item from ItemSample and amount
    */
    Item(const ItemSample& sample, int amount);

    /**
     * @brief Constructs item from its ID amount and special value
    */
    Item(ItemId id, int amount, float special = 0.0f);

        //Merge with another item of same ID and special (checks are done inside this function)
        //Both items may be changed
        //Action is done only with partion of item
    void merge(Item& item, float portion);
        //Insert another item into this item if this item is empty (checks are done inside this function)
        //Action is done only with partion of item
    void insert(Item& item, float portion);
        //Swaps contents of two items
    void swap(Item& item);

    /**
     * @brief Reduces amount by one
     * @return The previous amount
    */
    int operator--();

    /**
     * @brief Reduces amount by one
     * @return The current amount
    */
    int operator--(int);

    /**
     * @brief Adds given number to amount
     * @return The current amount
    */
    int operator+=(int number);

    /**
     * @brief Removes given number from amount
     * @return The current amount
    */
    int operator-=(int number);
    /**
     * @brief Multiplies number by given amount
     * @return The current amount
    */
    Item operator*(int number) const;


    int amount = 0;
};