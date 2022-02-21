#pragma once
#include <string>
#include <vector>
#include <cmath>

const std::string ITEM_ATLAS_PREFIX = "itemAtlas";

enum class I_ID : uint16_t {
	EMPTY,

	B_WATER,
	B_LAVA,
	B_ACID,
	B_STONE,
	B_DIRT,
	B_GRASS,
	B_COLD_STONE,
	B_SAND,
	B_COLD_DIRT,
	B_COLD_GRASS,
	B_MUD,
	B_MUD_GRASS,
	B_DRY_GRASS,

	W_STONE,
	W_DIRT,
	W_GRASS,
	W_COLD_STONE,
	W_SAND,
	W_COLD_DIRT,
	W_COLD_GRASS,
	W_MUD,
	W_MUD_GRASS,
	W_DRY_GRASS,

	WOODEN_PICKAXE,
	STONE_PICKAXE,
	BRONZE_PICKAXE,
	STEEL_PICKAXE,

	WOODEN_HAMMER,
	STONE_HAMMER,
	BRONZE_HAMMER,
	STEEL_HAMMER,

	LAST
};

enum class I_TYPE : uint32_t {
	EMPTY,
	MATERIAL,
	BLOCK,
	WALL,
	PICKAXE,//Mines blocks
	HAMMER,//Mines walls
	CHISEL
};

#pragma warning(push)
#pragma warning(disable: 26495)
struct ItemMetadata {
	ItemMetadata() {};
	ItemMetadata(const std::string& name, int32_t maxStack, char textureAtlas, float spriteIndex, float drawScale, I_TYPE type, int32_t typeIndex) :
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
	int32_t maxStack/* = 0*/;

	char textureAtlas/* = '0'*/;//Indicates which texture atlas should be used to draw this item
	float spriteIndex/* = 0.0f*/;//Indicates which sprite in the texture atlas should be used
	float drawScale;//Both X and Y scaling used when drawing the item inside slot (in world it is unstretched)

	I_TYPE type/* = I_TYPE::MATERIAL*/;
	int32_t typeIndex/* = 0*/;//ID of the block with block types | totalIndex of furniture with furniture types
};
#pragma warning(pop)

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