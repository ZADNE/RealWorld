/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/items/ItemSample.hpp>

namespace rw {

/**
 * @brief Representes a number of item samples.
 */
struct Item: public ItemSample {

    /**
     * @brief Constructs empty item
     */
    Item() = default;

    /**
     * @brief Constructs item from ItemSample and count
     */
    Item(const ItemSample& sample, int count);

    /**
     * @brief Constructs item from its ID count and special value
     */
    Item(ItemId id, int count, float special = 0.0f);

    // Merge with another item of same ID and special (checks are done inside
    // this function) Both items may be changed Action is done only with partion
    // of item
    void merge(Item& item, float portion);
    // Insert another item into this item if this item is empty (checks are done
    // inside this function) Action is done only with partion of item
    void insert(Item& item, float portion);

    /**
     * @brief Reduces count by one
     * @return The previous count
     */
    int operator--();

    /**
     * @brief Reduces count by one
     * @return The current count
     */
    int operator--(int);

    /**
     * @brief Adds given number to count
     * @return The current count
     */
    int operator+=(int number);

    /**
     * @brief Removes given number from count
     * @return The current count
     */
    int operator-=(int number);
    /**
     * @brief Multiplies number by given count
     * @return The current count
     */
    Item operator*(int number) const;

    int count = 0;
};

} // namespace rw
