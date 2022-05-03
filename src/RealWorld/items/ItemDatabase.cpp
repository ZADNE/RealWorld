#include <RealWorld/items/ItemDatabase.hpp>

using enum ITEM_TYPE;
const std::array<ItemMetadata, magic_enum::enum_count<ITEM>()> ItemDatabase::m_itemMetadata = {
	//			maxStack	texAtlas	spriteIndex		drawScale	type		typeIndex	name
	ItemMetadata{0,			'B',		0.0f,			1.0f,		EMPTY,		0,			"Empty"},
	//Fluids
	ItemMetadata{1000,		'F',		0.0f,			4.0f,		BLOCK,		1,			"Water"},
	ItemMetadata{1000,		'F',		1.0f,			4.0f,		BLOCK,		2,			"Lava"},
	ItemMetadata{1000,		'F',		2.0f,			4.0f,		BLOCK,		3,			"Steam"},
	ItemMetadata{1000,		'F',		3.0f,			4.0f,		BLOCK,		4,			"Fire"},
	ItemMetadata{1000,		'F',		4.0f,			4.0f,		BLOCK,		5,			"Smoke"},
	ItemMetadata{1000,		'F',		5.0f,			4.0f,		BLOCK,		15,			"Acid"},
	//Blocks
	ItemMetadata{1000,		'B',		0.0f,			4.0f,		BLOCK,		16,			"Stone Block"},
	ItemMetadata{1000,		'B',		1.0f,			4.0f,		BLOCK,		17,			"Dirt Block"},
	ItemMetadata{1000,		'B',		2.0f,			4.0f,		BLOCK,		18,			"Grass Block"},
	ItemMetadata{1000,		'B',		3.0f,			4.0f,		BLOCK,		19,			"Cold Stone Block"},
	ItemMetadata{1000,		'B',		4.0f,			4.0f,		BLOCK,		20,			"Sand Block"},
	ItemMetadata{1000,		'B',		5.0f,			4.0f,		BLOCK,		21,			"Cold Dirt Block"},
	ItemMetadata{1000,		'B',		6.0f,			4.0f,		BLOCK,		22,			"Cold Grass Block"},
	ItemMetadata{1000,		'B',		7.0f,			4.0f,		BLOCK,		23,			"Mud Block"},
	ItemMetadata{1000,		'B',		8.0f,			4.0f,		BLOCK,		24,			"Mud Grass Block"},
	ItemMetadata{1000,		'B',		9.0f,			4.0f,		BLOCK,		25,			"Dry Grass Block"},
	//Walls
	ItemMetadata{1000,		'W',		0.0f,			4.0f,		WALL,		1,			"Stone Wall"},
	ItemMetadata{1000,		'W',		1.0f,			4.0f,		WALL,		2,			"Dirt Wall"},
	ItemMetadata{1000,		'W',		2.0f,			4.0f,		WALL,		3,			"Grass Wall"},
	ItemMetadata{1000,		'W',		3.0f,			4.0f,		WALL,		4,			"Cold Stone Wall"},
	ItemMetadata{1000,		'W',		4.0f,			4.0f,		WALL,		5,			"Sand Wall"},
	ItemMetadata{1000,		'W',		5.0f,			4.0f,		WALL,		6,			"Cold Dirt Wall"},
	ItemMetadata{1000,		'W',		6.0f,			4.0f,		WALL,		7,			"Cold Grass Wall"},
	ItemMetadata{1000,		'W',		7.0f,			4.0f,		WALL,		8,			"Mud Wall"},
	ItemMetadata{1000,		'W',		8.0f,			4.0f,		WALL,		9,			"Mud Grass Wall"},
	ItemMetadata{1000,		'W',		9.0f,			4.0f,		WALL,		10,			"Dry Grass Wall"},
	//Pickaxes
	ItemMetadata{1000,		'P',		0.0f,			1.0f,		PICKAXE,	0,			"Pickaxe"},
	//Hammers
	ItemMetadata{1000,		'H',		0.0f,			1.0f,		HAMMER,		0,			"Hammer"}
};

const ItemMetadata& ItemDatabase::md(ITEM ID) {
	return m_itemMetadata[static_cast<int>(ID)];
}