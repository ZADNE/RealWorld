#pragma once
#include <string>
#include <array>

#include <magic_enum/magic_enum.hpp>

enum class ITEM : uint16_t {
	EMPTY,
	//Fluids
	F_WATER,
	F_LAVA,
	F_STEAM,
	F_FIRE,
	F_SMOKE,
	F_ACID,
	//Blocks
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
	B_HALLOW_STONE,
	B_HALLOW_DIRT,
	B_HALLOW_GRASS,
	//Walls
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
	//Pickaxes
	CREATIVE_PICKAXE,
	///Hammers
	CREATIVE_HAMMER,
};

enum class ITEM_TYPE : uint32_t {
	EMPTY,
	MATERIAL,
	BLOCK,
	WALL,
	PICKAXE,//Mines blocks
	HAMMER,//Mines walls
	CURSOR_LIGHT,
	OPERATOR_LIGHT
};

struct ItemMetadata {
	ItemMetadata(int maxStack, char textureAtlas, float spriteIndex, float drawScale, ITEM_TYPE type, int typeIndex, const std::string& name) :
		name(name),
		maxStack(maxStack),
		textureAtlas(textureAtlas),
		spriteIndex(spriteIndex),
		drawScale(drawScale),
		type(type),
		typeIndex(typeIndex) {

	};

	std::string name;
	int maxStack;

	char textureAtlas;//Determines which texture atlas should be used to draw this item
	float spriteIndex;//Determines which sprite in the texture atlas should be used
	float drawScale;//Both X and Y scaling used when drawing the item inside slot

	ITEM_TYPE type;
	int typeIndex;//ID of the block with block types
};

/**
 * @brief Is a readonly database of item metadata
*/
class ItemDatabase {
public:

	/**
	 * @brief Fetches metadata of an item
	*/
	const static ItemMetadata& md(ITEM ID);
private:

	const static std::array<ItemMetadata, magic_enum::enum_count<ITEM>()> m_itemMetadata;
};