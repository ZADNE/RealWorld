#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/metadata.hpp>
#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/items/IDB.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemOnGroundManager.hpp>
#include <RealWorld/furniture/FurnitureManager.hpp>

enum ItemUse { MAIN, ALTERNATIVE, NUMBER_OF_USES };

class ItemUser {
public:
	ItemUser(World& world, FurnitureManager& furnitureManager, Inventory& inventory, Hitbox& operatorsHitbox, RE::SpriteBatch& spriteBatch, ItemOnGroundManager& itemOnGroundManager);
	~ItemUser();

	void beginUse(ItemUse use);
	void endUse(ItemUse use);
	//Does not check if the slot is inside the inventory!
	void chooseSlot(int slot);

	void step(const glm::ivec2& relCursorPos);
	void draw();
private:
	World& m_world;
	FurnitureManager& m_furnitureManager;
	Inventory& m_inv;
	Hitbox& m_operatorsHitbox;
	ItemOnGroundManager& m_itemOnGroundManager;
	RE::SpriteBatch& m_spriteBatch;

	int m_chosenSlot = 0;

	// <  0: steps not using
	// == 0: just stopped using
	// >  0: steps using
	// == 1: just started using
	int m_using[ItemUse::NUMBER_OF_USES] = {false, false};
	Item* m_item = nullptr;

	glm::ivec2 m_relCursorPos;//Relative position of the cursor to the world view

		//Furniture-building related
	unsigned int m_canBuildFurniture = 0;//0 = too far, 1 = bad placement/occupied, 2 = can build
	RE::Colour m_furBlueprint[3] = {{0, 0, 0, 0}, {255, 63, 63, 191}, {63, 63, 255, 191}};
	void checkBuildFurniture();


	//TILE RELATED \|/
	void reloadTarget();
	float m_buildingRange = 8.0f * TILE_SIZE.x;
	//UC -> under cursor
	glm::ivec2 m_UCTileBc = glm::ivec2(0, 0);//Position in blocks
	glm::ivec2 m_UCTilePx = glm::ivec2(0, 0);//Center of the block in pixels
	BLOCK m_UCBlock = BLOCK::AIR;
	WALL m_UCWall = WALL::AIR;
	//P -> previous (beginStep)
	glm::ivec2 m_UCTileBcP = glm::ivec2(0, 0);//Position in blocks
	glm::ivec2 m_UCTilePxP = glm::ivec2(0, 0);//Center of the block in pixels
	BLOCK m_UCBlockP = BLOCK::AIR;
	WALL m_UCWallP = WALL::AIR;

	float m_neededToMineBlock = 0.0f;
	float m_neededToMineWall = 0.0f;

	RE::TexturePtr m_miningBlockTex = RE::RM::getTexture("miningBlock");
};
