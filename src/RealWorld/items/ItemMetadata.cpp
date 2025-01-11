/**
 *  @author    Dubsky Tomas
 */
#include <cassert>

#include <RealWorld/constants/ResourceIndex.hpp>
#include <RealWorld/items/ItemMetadata.hpp>

namespace rw {

// NOLINTBEGIN(*-magic-numbers): This is the place to define the numbers

ItemIDSection section(ItemID id) {
    return static_cast<ItemIDSection>(
        std::to_underlying(id) & std::to_underlying(ItemIDSection::Mask)
    );
}

uint16_t offsetInSection(ItemID id) { // Offset within section
    return std::to_underlying(id) & ~std::to_underlying(ItemIDSection::Mask);
}

int maxStack(ItemID id) {
    switch (section(id)) {
    case ItemIDSection::Mixed:    return 0;
    case ItemIDSection::Blocks:   return 100'000;
    case ItemIDSection::Walls:    return 100'000;
    case ItemIDSection::Pickaxes: return 1;
    case ItemIDSection::Hammers:  return 1;
    default:                      std::unreachable();
    }
}

re::ResourceID textureID(ItemID id) {
    switch (section(id)) {
    case ItemIDSection::Mixed: return textureID<"itemAtlasB">();
    case ItemIDSection::Blocks:
        return offsetInSection(id) >= std::to_underlying(Block::FirstNonsolid)
                   ? textureID<"itemAtlasF">()
                   : textureID<"itemAtlasB">();
    case ItemIDSection::Walls:    return textureID<"itemAtlasW">();
    case ItemIDSection::Pickaxes: return textureID<"itemAtlasP">();
    case ItemIDSection::Hammers:  return textureID<"itemAtlasH">();
    default:                      std::unreachable();
    }
}

float spriteIndex(ItemID id) {
    auto offset = offsetInSection(id);
    switch (section(id)) {
    case ItemIDSection::Mixed: return offset;
    case ItemIDSection::Blocks:
        return offset >= std::to_underlying(Block::FirstNonsolid)
                   ? offset & k_nonSolidsMask
                   : offset;
    case ItemIDSection::Walls:    return offset;
    case ItemIDSection::Pickaxes: return offset;
    case ItemIDSection::Hammers:  return offset;
    default:                      std::unreachable();
    }
}

float subimageIndex(ItemID id) {
    return 0.0f;
}

float drawScale(ItemID id) {
    switch (section(id)) {
    case ItemIDSection::Mixed:    return 1.0f;
    case ItemIDSection::Blocks:   return 4.0f;
    case ItemIDSection::Walls:    return 4.0f;
    case ItemIDSection::Pickaxes: return 1.0f;
    case ItemIDSection::Hammers:  return 1.0f;
    default:                      std::unreachable();
    }
}

// NOLINTEND(*-magic-numbers)

} // namespace rw
