/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealWorld/items/ItemMetadata.hpp>

namespace rw {

/**
 * @brief Is an instance of a type of item.
 */
struct ItemSample {
    ItemSample() {}
    ItemSample(ItemID id, float special)
        : id(id)
        , special(special) {}

    friend auto operator<=>(const ItemSample&, const ItemSample&) = default;

    // ItemSample is considered empty if its ItemID == ItemID::Empty
    bool isEmpty() const { return id == ItemID::Empty; }

    ItemID id     = ItemID::Empty;
    float special = 0.0f;
};

struct ItemSampleHasher {
    size_t operator()(const ItemSample& otherSample) const {
        return ((size_t)otherSample.id | (size_t)otherSample.special << 16);
    }
};

} // namespace rw
