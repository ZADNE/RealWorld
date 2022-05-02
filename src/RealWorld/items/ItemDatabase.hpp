#pragma once
#include <string>
#include <vector>
#include <cmath>

enum class ITEM : uint16_t {
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

	CREATIVE_PICKAXE,

	CREATIVE_HAMMER,

	LAST
};

enum class ITEM_TYPE : uint32_t {
	EMPTY,
	MATERIAL,
	BLOCK,
	WALL,
	PICKAXE,//Mines blocks
	HAMMER,//Mines walls
};

#pragma warning(push)
#pragma warning(disable: 26495)
struct ItemMetadata {
	ItemMetadata() {};
	ItemMetadata(const std::string& name, int32_t maxStack, char textureAtlas, float spriteIndex, float drawScale, ITEM_TYPE type, int32_t typeIndex) :
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

	ITEM_TYPE type/* = ITEM_TYPE::MATERIAL*/;
	int32_t typeIndex/* = 0*/;//ID of the block with block types | totalIndex of furniture with furniture types
};
#pragma warning(pop)

/**
 * @brief Is a readonly database of item metadata
*/
class ItemDatabase {
public:
	/**
	 * @brief To be called once at startup
	*/
	void static init();

	/**
	 * @brief Fetches metadata of an item
	*/
	const static ItemMetadata& md(ITEM ID);
private:

	static std::vector<ItemMetadata> m_itemMetadata;
};