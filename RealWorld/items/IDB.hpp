#pragma once
#include <string>
#include <vector>
#include <cmath>

#include <RealWorld/metadata.hpp>

const std::string ITEM_ATLAS_PREFIX = "itemAtlas";

enum class I_ID : ushort {
	EMPTY,

	B_STONE,
	B_DIRT,
	B_GRASS,
	B_COAL,
	B_HEMATITE,
	B_BAUXITE,
	B_COPPER_ORE,
	B_CASSITERITE,
	B_GOLD_ORE,
	B_SPODUMENE,
	B_QUARTZ_SANDSTONE,
	B_BRICK,
	B_RAW_WOOD,
	B_WOODEN_PLANKS,

	W_STONE,
	W_GRASS,

	WOODEN_PICKAXE,
	STONE_PICKAXE,
	BRONZE_PICKAXE,
	STEEL_PICKAXE,

	WOODEN_HAMMER,
	STONE_HAMMER,
	BRONZE_HAMMER,
	STEEL_HAMMER,

	F_CHEST,
	F_CHANDELIER,

	LAST
};

enum class I_TYPE : ulong {
	EMPTY,
	MATERIAL,
	BLOCK,
	WALL,
	PICKAXE,//Mines blocks
	HAMMER,//Mines walls
	CHISEL,
	FURNITURE
};

struct ItemMetadata {
	ItemMetadata() {};
	ItemMetadata(const std::string& name, int maxStack, char textureAtlas, float spriteIndex, float drawScale, I_TYPE type, int typeIndex) :
		name(name),
		maxStack(maxStack),
		textureAtlas(textureAtlas),
		spriteIndex(std::floor(spriteIndex)),
		drawScale(drawScale),
		type(type),
		typeIndex(typeIndex) {

	};

	static size_t constexpr saveSize() { return (sizeof(maxStack) + sizeof(textureAtlas) + sizeof(spriteIndex) + sizeof(drawScale) + sizeof(type) + sizeof(typeIndex)); }

	std::string name/* = ""*/;
	int maxStack/* = 0*/;

	char textureAtlas/* = '0'*/;//Indicates which texture atlas should be used to draw this item
	float spriteIndex/* = 0.0f*/;//Indicates which sprite in the texture atlas should be used
	float drawScale;//Both X and Y scaling used when drawing the item inside slot (in world it is unstretched)

	I_TYPE type/* = I_TYPE::MATERIAL*/;
	int typeIndex/* = 0*/;//ID of the block with block types | totalIndex of furniture with furniture types
};

struct PickaxeMetadata {
	PickaxeMetadata(int strength, float speed, float range) :
		strength(strength),
		speed(speed),
		range(range) {

	};

	int strength;//How hard blocks it can mine, must be equal or higher than block's hardness to mine it
	float speed;//How fast it mines
	float range;//How distant blocks it can mine (in pixels)
};

struct HammerMetadata {
	HammerMetadata(int strength, float speed, float range) :
		strength(strength),
		speed(speed),
		range(range) {

	};

	int strength;//How hard blocks it can mine, must be equal or higher than wall's hardness to mine it
	float speed;//How fast it mines
	float range;//How distant walls it can mine (in pixels)
};

PickaxeMetadata const pickaxeMetadata[4] = {
	PickaxeMetadata(100,	1.0f,	128.0f),
	PickaxeMetadata(100,	0.9f,	128.0f),
	PickaxeMetadata(100,	1.0f,	128.0f),
	PickaxeMetadata(100,	100.0f,	128.0f)
};

HammerMetadata const hammerMetadata[4] = {
	HammerMetadata(100,		0.8f,	128.0f),
	HammerMetadata(100,		0.9f,	128.0f),
	HammerMetadata(100,		1.0f,	128.0f),
	HammerMetadata(100,		1.1f,	128.0f)
};

class IDB {
public:
	void static init();

	const static ItemMetadata& g(I_ID ID);
private:

	static std::vector<ItemMetadata> m_itemMetadata;
};