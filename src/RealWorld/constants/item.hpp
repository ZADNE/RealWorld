/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <utility>

#include <RealWorld/constants/tile.hpp>

namespace rw {

enum class ItemIdSection : uint16_t {
    Mask     = 0xff00,
    Mixed    = 0x0000,
    Blocks   = 0x0100,
    Walls    = 0x0200,
    Pickaxes = 0x0300,
    Hammers  = 0x0400
};

enum class ItemId : uint16_t;

constexpr std::underlying_type<ItemId>::type toItemId(Block block) {
    return std::to_underlying(ItemIdSection::Blocks) + std::to_underlying(block);
}

constexpr std::underlying_type<ItemId>::type toItemId(Wall wall) {
    return std::to_underlying(ItemIdSection::Walls) + std::to_underlying(wall);
}

/**
 * @brief Item ids are divided into sections of 256 ids
 * @see rw::ItemIdSection
 */
enum class ItemId : uint16_t {
    // Mixed section
    Empty,
    // Block section
    BlockSection    = std::to_underlying(ItemIdSection::Blocks),
    BStone          = toItemId(Block::Stone),
    BDirt           = toItemId(Block::Dirt),
    BGrass          = toItemId(Block::Grass),
    BColdStone      = toItemId(Block::ColdStone),
    BSand           = toItemId(Block::Sand),
    BColdDirt       = toItemId(Block::ColdDirt),
    BColdGrass      = toItemId(Block::ColdGrass),
    BMud            = toItemId(Block::Mud),
    BMudGrass       = toItemId(Block::MudGrass),
    BDryGrass       = toItemId(Block::DryGrass),
    BHallowStone    = toItemId(Block::HallowStone),
    BHallowDirt     = toItemId(Block::HallowDirt),
    BHallowGrass    = toItemId(Block::HallowGrass),
    BHighlighter    = toItemId(Block::Highlighter),
    BWater          = toItemId(Block::Water),
    BLava           = toItemId(Block::Lava),
    BSteam          = toItemId(Block::Steam),
    BFire           = toItemId(Block::Fire),
    BSmoke          = toItemId(Block::Smoke),
    BDroppedLeaf    = toItemId(Block::DroppedLeaf),
    BDroppedDryLeaf = toItemId(Block::DroppedDryLeaf),
    BAcid           = toItemId(Block::Acid),
    // Wall section
    WallSection     = std::to_underlying(ItemIdSection::Walls),
    WStone          = toItemId(Wall::Stone),
    WDirt           = toItemId(Wall::Dirt),
    WGrass          = toItemId(Wall::Grass),
    WColdStone      = toItemId(Wall::ColdStone),
    WSand           = toItemId(Wall::Sand),
    WColdDirt       = toItemId(Wall::ColdDirt),
    WColdGrass      = toItemId(Wall::ColdGrass),
    WMud            = toItemId(Wall::Mud),
    WMudGrass       = toItemId(Wall::MudGrass),
    WDryGrass       = toItemId(Wall::DryGrass),
    WHallowStone    = toItemId(Wall::HallowStone),
    WHallowDirt     = toItemId(Wall::HallowDirt),
    WHallowGrass    = toItemId(Wall::HallowGrass),
    WHighlighter    = toItemId(Wall::Highlighter),
    WOakWood        = toItemId(Wall::OakWood),
    WAcaciaWood     = toItemId(Wall::AcaciaWood),
    WConiferousWood = toItemId(Wall::ConiferousWood),
    WTallGrass      = toItemId(Wall::TallGrass),
    WColdTallGrass  = toItemId(Wall::ColdTallGrass),
    WMudTallGrass   = toItemId(Wall::MudTallGrass),
    WDryTallGrass   = toItemId(Wall::DryTallGrass),
    WLeaf           = toItemId(Wall::Leaf),
    WDryLeaf        = toItemId(Wall::DryLeaf),
    WNeedle         = toItemId(Wall::Needle),
    WPalmLeaf       = toItemId(Wall::PalmLeaf),
    WWithy          = toItemId(Wall::Withy),
    WCactus         = toItemId(Wall::Cactus),
    WBurningWood    = toItemId(Wall::BurningWood),
    WBurntWood      = toItemId(Wall::BurntWood),
    WHallowWood     = toItemId(Wall::HallowWood),
    // Pickaxe section
    PickaxeSection   = std::to_underlying(ItemIdSection::Pickaxes),
    PCreativePickaxe = PickaxeSection,
    // Hammer section
    HammerSection   = std::to_underlying(ItemIdSection::Hammers),
    HCreativeHammer = HammerSection,
};

/**
 * @brief Default size in slots of player's inventory
 */
constexpr glm::ivec2 k_defaultPlayerInventorySize = glm::ivec2{10, 4};

} // namespace rw
