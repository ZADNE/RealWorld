/**
 *  @author    Dubsky Tomas
 */
#include <cassert>

#include <RealWorld/constants/ResourceIndex.hpp>
#include <RealWorld/items/ItemMetadata.hpp>

namespace rw {

// NOLINTBEGIN(*-magic-numbers): This is the place to define the numbers

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
    default:                      std::unreachable();
    }
}

re::ResourceID textureID(ItemId id) {
    switch (section(id)) {
    case ItemIdSection::Mixed: return textureID<"itemAtlasB">();
    case ItemIdSection::Blocks:
        return offsetInSection(id) >= std::to_underlying(Block::FirstNonsolid)
                   ? textureID<"itemAtlasF">()
                   : textureID<"itemAtlasB">();
    case ItemIdSection::Walls:    return textureID<"itemAtlasW">();
    case ItemIdSection::Pickaxes: return textureID<"itemAtlasP">();
    case ItemIdSection::Hammers:  return textureID<"itemAtlasH">();
    default:                      std::unreachable();
    }
}

float spriteIndex(ItemId id) {
    auto offset = offsetInSection(id);
    switch (section(id)) {
    case ItemIdSection::Mixed: return offset;
    case ItemIdSection::Blocks:
        return offset >= std::to_underlying(Block::FirstNonsolid)
                   ? offset & k_nonSolidsMask
                   : offset;
    case ItemIdSection::Walls:    return offset;
    case ItemIdSection::Pickaxes: return offset;
    case ItemIdSection::Hammers:  return offset;
    default:                      std::unreachable();
    }
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
    default:                      std::unreachable();
    }
}

// NOLINTEND(*-magic-numbers)

} // namespace rw
