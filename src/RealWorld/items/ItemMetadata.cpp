/*!
 *  @author    Dubsky Tomas
 */
#include <cassert>

#include <RealWorld/items/ItemMetadata.hpp>

namespace rw {

ItemIdSection section(ItemId id) {
    return static_cast<ItemIdSection>(
        std::to_underlying(id) & std::to_underlying(ItemIdSection::Mask)
    );
}

uint16_t offsetInSection(ItemId id) { // Offset within section
    return std::to_underlying(id) & ~std::to_underlying(ItemIdSection::Mask);
}

int maxStack(ItemId id) {
    switch (section(id)) {
    case ItemIdSection::Mixed:    return 0;
    case ItemIdSection::Blocks:   return 100'000;
    case ItemIdSection::Walls:    return 100'000;
    case ItemIdSection::Pickaxes: return 1;
    case ItemIdSection::Hammers:  return 1;
    default:                      assert(!"Unknown item"); std::unreachable();
    }
}

char textureAtlas(ItemId id) {
    switch (section(id)) {
    case ItemIdSection::Mixed:    return 'B';
    case ItemIdSection::Blocks:   return 'B';
    case ItemIdSection::Walls:    return 'W';
    case ItemIdSection::Pickaxes: return 'P';
    case ItemIdSection::Hammers:  return 'H';
    default:                      assert(!"Unknown item"); std::unreachable();
    }
}

float spriteIndex(ItemId id) {
    return offsetInSection(id);
}

float subimageIndex(ItemId id) {
    return 0.0f;
}

float drawScale(ItemId id) {
    switch (section(id)) {
    case ItemIdSection::Mixed:    return 1.0f;
    case ItemIdSection::Blocks:   return 4.0f;
    case ItemIdSection::Walls:    return 4.0f;
    case ItemIdSection::Pickaxes: return 1.0f;
    case ItemIdSection::Hammers:  return 1.0f;
    default:                      assert(!"Unknown item"); std::unreachable();
    }
}

} // namespace rw
