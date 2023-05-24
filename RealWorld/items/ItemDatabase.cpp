/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemDatabase.hpp>

#include <RealWorld/constants/tile.hpp>

template<typename T>
int i(T enumm) {
    return static_cast<int>(enumm);
}

const std::array<ItemMetadata, static_cast<size_t>(ItemId::Count)> ItemDatabase::m_itemMetadata = {
    //Empty     maxStack    texAtlas    spriteIndex     drawScale    type                       typeIndex                   name
    ItemMetadata{0,         'B',        0.0f,           1.0f,        ItemType::Empty,          0,                           "Empty"},
    //Fluids    maxStack    texAtlas    spriteIndex     drawScale    type                       typeIndex                   name
    ItemMetadata{1000,      'F',        0.0f,           4.0f,        ItemType::Block,          i(Block::Water),             "Water"},
    ItemMetadata{1000,      'F',        1.0f,           4.0f,        ItemType::Block,          i(Block::Lava),              "Lava"},
    ItemMetadata{1000,      'F',        2.0f,           4.0f,        ItemType::Block,          i(Block::Steam),             "Steam"},
    ItemMetadata{1000,      'F',        3.0f,           4.0f,        ItemType::Block,          i(Block::Fire),              "Fire"},
    ItemMetadata{1000,      'F',        4.0f,           4.0f,        ItemType::Block,          i(Block::Smoke),             "Smoke"},
    ItemMetadata{1000,      'F',        5.0f,           4.0f,        ItemType::Block,          i(Block::Acid),              "Acid"},
    //Blocks    maxStack    texAtlas    spriteIndex     drawScale    type                       typeIndex                   name
    ItemMetadata{1000,      'B',        0.0f,           4.0f,        ItemType::Block,          i(Block::Stone),             "Stone Block"},
    ItemMetadata{1000,      'B',        1.0f,           4.0f,        ItemType::Block,          i(Block::Dirt),              "Dirt Block"},
    ItemMetadata{1000,      'B',        2.0f,           4.0f,        ItemType::Block,          i(Block::Grass),             "Grass Block"},
    ItemMetadata{1000,      'B',        3.0f,           4.0f,        ItemType::Block,          i(Block::ColdStone),         "Cold Stone Block"},
    ItemMetadata{1000,      'B',        4.0f,           4.0f,        ItemType::Block,          i(Block::Sand),              "Sand Block"},
    ItemMetadata{1000,      'B',        5.0f,           4.0f,        ItemType::Block,          i(Block::ColdDirt),          "Cold Dirt Block"},
    ItemMetadata{1000,      'B',        6.0f,           4.0f,        ItemType::Block,          i(Block::ColdGrass),         "Cold Grass Block"},
    ItemMetadata{1000,      'B',        7.0f,           4.0f,        ItemType::Block,          i(Block::Mud),               "Mud Block"},
    ItemMetadata{1000,      'B',        8.0f,           4.0f,        ItemType::Block,          i(Block::MudGrass),          "Mud Grass Block"},
    ItemMetadata{1000,      'B',        9.0f,           4.0f,        ItemType::Block,          i(Block::DryGrass),          "Dry Grass Block"},
    ItemMetadata{1000,      'B',        10.0f,          4.0f,        ItemType::Block,          i(Block::HallowStone),       "Hallow Stone Block"},
    ItemMetadata{1000,      'B',        11.0f,          4.0f,        ItemType::Block,          i(Block::HallowDirt),        "Hallow Dirt Block"},
    ItemMetadata{1000,      'B',        12.0f,          4.0f,        ItemType::Block,          i(Block::HallowGrass),       "Hallow Grass Block"},
    //Walls     maxStack    texAtlas    spriteIndex     drawScale    type                       typeIndex                   name
    ItemMetadata{1000,      'W',        0.0f,           4.0f,        ItemType::Wall,           i(Wall::Stone),              "Stone Wall"},
    ItemMetadata{1000,      'W',        1.0f,           4.0f,        ItemType::Wall,           i(Wall::Dirt),               "Dirt Wall"},
    ItemMetadata{1000,      'W',        2.0f,           4.0f,        ItemType::Wall,           i(Wall::Grass),              "Grass Wall"},
    ItemMetadata{1000,      'W',        3.0f,           4.0f,        ItemType::Wall,           i(Wall::ColdStone),          "Cold Stone Wall"},
    ItemMetadata{1000,      'W',        4.0f,           4.0f,        ItemType::Wall,           i(Wall::Sand),               "Sand Wall"},
    ItemMetadata{1000,      'W',        5.0f,           4.0f,        ItemType::Wall,           i(Wall::ColdDirt),           "Cold Dirt Wall"},
    ItemMetadata{1000,      'W',        6.0f,           4.0f,        ItemType::Wall,           i(Wall::ColdGrass),          "Cold Grass Wall"},
    ItemMetadata{1000,      'W',        7.0f,           4.0f,        ItemType::Wall,           i(Wall::Mud),                "Mud Wall"},
    ItemMetadata{1000,      'W',        8.0f,           4.0f,        ItemType::Wall,           i(Wall::MudGrass),           "Mud Grass Wall"},
    ItemMetadata{1000,      'W',        9.0f,           4.0f,        ItemType::Wall,           i(Wall::DryGrass),           "Dry Grass Wall"},
    //Pickaxes  maxStack    texAtlas    spriteIndex     drawScale    type                       typeIndex                   name
    ItemMetadata{1000,      'P',        0.0f,           1.0f,        ItemType::Pickaxe,        0,                           "Pickaxe"},
    //Hammers   maxStack    texAtlas    spriteIndex     drawScale    type                       typeIndex                   name
    ItemMetadata{1000,      'H',        0.0f,           1.0f,        ItemType::Hammer,         0,                           "Hammer"}
};

const ItemMetadata& ItemDatabase::md(ItemId id) {
    return m_itemMetadata[static_cast<int>(id)];
}