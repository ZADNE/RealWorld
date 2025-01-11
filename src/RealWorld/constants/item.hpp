/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <utility>

#include <RealWorld/constants/tile.hpp>

namespace rw {

enum class ItemIDSection : uint16_t {
    Mask     = 0xff00,
    Mixed    = 0x0000,
    Blocks   = 0x0100,
    Walls    = 0x0200,
    Pickaxes = 0x0300,
    Hammers  = 0x0400
};

enum class ItemID : uint16_t;

constexpr std::underlying_type<ItemID>::type toItemID(Block block) {
    return std::to_underlying(ItemIDSection::Blocks) + std::to_underlying(block);
}

constexpr std::underlying_type<ItemID>::type toItemID(Wall wall) {
    return std::to_underlying(ItemIDSection::Walls) + std::to_underlying(wall);
}

/**
 * @brief Item ids are divided into sections of 256 ids
 * @see rw::ItemIDSection
 */
enum class ItemID : uint16_t {
    // Mixed section
    Empty,
    // Block section
    BlockSection    = std::to_underlying(ItemIDSection::Blocks),
    BStone          = toItemID(Block::Stone),
    BDirt           = toItemID(Block::Dirt),
    BGrass          = toItemID(Block::Grass),
    BColdStone      = toItemID(Block::ColdStone),
    BSand           = toItemID(Block::Sand),
    BColdDirt       = toItemID(Block::ColdDirt),
    BColdGrass      = toItemID(Block::ColdGrass),
    BMud            = toItemID(Block::Mud),
    BMudGrass       = toItemID(Block::MudGrass),
    BDryGrass       = toItemID(Block::DryGrass),
    BHallowStone    = toItemID(Block::HallowStone),
    BHallowDirt     = toItemID(Block::HallowDirt),
    BHallowGrass    = toItemID(Block::HallowGrass),
    BHighlighter    = toItemID(Block::Highlighter),
    BWater          = toItemID(Block::Water),
    BLava           = toItemID(Block::Lava),
    BSteam          = toItemID(Block::Steam),
    BFire           = toItemID(Block::Fire),
    BSmoke          = toItemID(Block::Smoke),
    BDroppedLeaf    = toItemID(Block::DroppedLeaf),
    BDroppedDryLeaf = toItemID(Block::DroppedDryLeaf),
    BAcid           = toItemID(Block::Acid),
    // Wall section
    WallSection     = std::to_underlying(ItemIDSection::Walls),
    WStone          = toItemID(Wall::Stone),
    WDirt           = toItemID(Wall::Dirt),
    WGrass          = toItemID(Wall::Grass),
    WColdStone      = toItemID(Wall::ColdStone),
    WSand           = toItemID(Wall::Sand),
    WColdDirt       = toItemID(Wall::ColdDirt),
    WColdGrass      = toItemID(Wall::ColdGrass),
    WMud            = toItemID(Wall::Mud),
    WMudGrass       = toItemID(Wall::MudGrass),
    WDryGrass       = toItemID(Wall::DryGrass),
    WHallowStone    = toItemID(Wall::HallowStone),
    WHallowDirt     = toItemID(Wall::HallowDirt),
    WHallowGrass    = toItemID(Wall::HallowGrass),
    WHighlighter    = toItemID(Wall::Highlighter),
    WOakWood        = toItemID(Wall::OakWood),
    WAcaciaWood     = toItemID(Wall::AcaciaWood),
    WConiferousWood = toItemID(Wall::ConiferousWood),
    WTallGrass      = toItemID(Wall::TallGrass),
    WColdTallGrass  = toItemID(Wall::ColdTallGrass),
    WMudTallGrass   = toItemID(Wall::MudTallGrass),
    WDryTallGrass   = toItemID(Wall::DryTallGrass),
    WLeaf           = toItemID(Wall::Leaf),
    WDryLeaf        = toItemID(Wall::DryLeaf),
    WNeedle         = toItemID(Wall::Needle),
    WPalmLeaf       = toItemID(Wall::PalmLeaf),
    WWithy          = toItemID(Wall::Withy),
    WCactus         = toItemID(Wall::Cactus),
    WBurningWood    = toItemID(Wall::BurningWood),
    WBurntWood      = toItemID(Wall::BurntWood),
    WHallowWood     = toItemID(Wall::HallowWood),
    // Pickaxe section
    PickaxeSection   = std::to_underlying(ItemIDSection::Pickaxes),
    PCreativePickaxe = PickaxeSection,
    // Hammer section
    HammerSection   = std::to_underlying(ItemIDSection::Hammers),
    HCreativeHammer = HammerSection,
};

/**
 * @brief Default size in slots of player's inventory
 */
constexpr glm::ivec2 k_defaultPlayerInventorySize = glm::ivec2{10, 4};

} // namespace rw
