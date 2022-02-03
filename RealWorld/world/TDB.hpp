#pragma once
#include <vector>

#include <RealWorld/metadata.hpp>
#include <RealWorld/items/IDB.hpp>


const unsigned int LAST_NONSOLIDBLOCK = 0;


struct BlockMetadata {
	BlockMetadata() {};
	BlockMetadata(int hardness, float toughness, I_ID itemID) :
		hardness(hardness),
		toughness(toughness),
		itemID(itemID) {

	};
	static size_t constexpr saveSize() { return (sizeof(hardness) + sizeof(toughness) + sizeof(itemID)); }

	int hardness = 100;//How strong pickaxe the player needs to mine it, strength of player's pickaxe must be equal or higher than block's hardness to mine it
	float toughness = 50.0f;//How long it will take to mine this
	I_ID itemID = I_ID::EMPTY;//ID of the item that is dropped when the block is mined
};

struct WallMetadata {
	WallMetadata() {};
	WallMetadata(int hardness, float toughness, I_ID itemID) :
		hardness(hardness),
		toughness(toughness),
		itemID(itemID) {

	};

	static size_t constexpr saveSize() { return (sizeof(hardness) + sizeof(toughness) + sizeof(itemID)); }

	int hardness = 100;//How strong hammer the player needs to mine it, strength of player's hammer must be equal or higher than wall's hardness to mine it
	float toughness = 50.0f;//How long it will take to mine this
	I_ID itemID = I_ID::EMPTY;//ID of the item that is dropped when the wall is mined
};

class TDB {
public:
	void static init();

	const static BlockMetadata& gb(BLOCK ID);
	const static WallMetadata& gw(WALL ID);
private:
	static std::vector<BlockMetadata> m_blockMetadata;
	static std::vector<WallMetadata> m_wallMetadata;
};