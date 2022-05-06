/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemDatabase.hpp>

#include <RealWorld/constants/tile.hpp>

template<typename T>
int i(T enumm) {
	return static_cast<int>(enumm);
}

const std::array<ItemMetadata, magic_enum::enum_count<ITEM>()> ItemDatabase::m_itemMetadata = {
	//Empty		maxStack	texAtlas	spriteIndex		drawScale	type					typeIndex				name
	ItemMetadata{0,			'B',		0.0f,			1.0f,		ITEM_TYPE::EMPTY,		0,						"Empty"},
	//Fluids	maxStack	texAtlas	spriteIndex		drawScale	type					typeIndex				name
	ItemMetadata{1000,		'F',		0.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::WATER),		"Water"},
	ItemMetadata{1000,		'F',		1.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::LAVA),			"Lava"},
	ItemMetadata{1000,		'F',		2.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::STEAM),		"Steam"},
	ItemMetadata{1000,		'F',		3.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::FIRE),			"Fire"},
	ItemMetadata{1000,		'F',		4.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::SMOKE),		"Smoke"},
	ItemMetadata{1000,		'F',		5.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::ACID),			"Acid"},
	//Blocks	maxStack	texAtlas	spriteIndex		drawScale	type					typeIndex				name
	ItemMetadata{1000,		'B',		0.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::STONE),		"Stone Block"},
	ItemMetadata{1000,		'B',		1.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::DIRT),			"Dirt Block"},
	ItemMetadata{1000,		'B',		2.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::GRASS),		"Grass Block"},
	ItemMetadata{1000,		'B',		3.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::COLD_STONE),	"Cold Stone Block"},
	ItemMetadata{1000,		'B',		4.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::SAND),			"Sand Block"},
	ItemMetadata{1000,		'B',		5.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::COLD_DIRT),	"Cold Dirt Block"},
	ItemMetadata{1000,		'B',		6.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::COLD_GRASS),	"Cold Grass Block"},
	ItemMetadata{1000,		'B',		7.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::MUD),			"Mud Block"},
	ItemMetadata{1000,		'B',		8.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::MUD_GRASS),	"Mud Grass Block"},
	ItemMetadata{1000,		'B',		9.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::DRY_GRASS),	"Dry Grass Block"},
	ItemMetadata{1000,		'B',		10.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::HALLOW_STONE),	"Hallow Stone Block"},
	ItemMetadata{1000,		'B',		11.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::HALLOW_DIRT),	"Hallow Dirt Block"},
	ItemMetadata{1000,		'B',		12.0f,			4.0f,		ITEM_TYPE::BLOCK,		i(BLOCK::HALLOW_GRASS),	"Hallow Grass Block"},
	//Walls		maxStack	texAtlas	spriteIndex		drawScale	type					typeIndex				name
	ItemMetadata{1000,		'W',		0.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::STONE),			"Stone Wall"},
	ItemMetadata{1000,		'W',		1.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::DIRT),			"Dirt Wall"},
	ItemMetadata{1000,		'W',		2.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::GRASS),			"Grass Wall"},
	ItemMetadata{1000,		'W',		3.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::COLD_STONE),	"Cold Stone Wall"},
	ItemMetadata{1000,		'W',		4.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::SAND),			"Sand Wall"},
	ItemMetadata{1000,		'W',		5.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::COLD_DIRT),		"Cold Dirt Wall"},
	ItemMetadata{1000,		'W',		6.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::COLD_GRASS),	"Cold Grass Wall"},
	ItemMetadata{1000,		'W',		7.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::MUD),			"Mud Wall"},
	ItemMetadata{1000,		'W',		8.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::MUD_GRASS),		"Mud Grass Wall"},
	ItemMetadata{1000,		'W',		9.0f,			4.0f,		ITEM_TYPE::WALL,		i(WALL::DRY_GRASS),		"Dry Grass Wall"},
	//Pickaxes	maxStack	texAtlas	spriteIndex		drawScale	type					typeIndex				name
	ItemMetadata{1000,		'P',		0.0f,			1.0f,		ITEM_TYPE::PICKAXE,		0,						"Pickaxe"},
	//Hammers	maxStack	texAtlas	spriteIndex		drawScale	type					typeIndex				name
	ItemMetadata{1000,		'H',		0.0f,			1.0f,		ITEM_TYPE::HAMMER,		0,						"Hammer"}
};

const ItemMetadata& ItemDatabase::md(ITEM ID) {
	return m_itemMetadata[static_cast<int>(ID)];
}