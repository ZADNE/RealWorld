#pragma once
#include <vector>

#include <RealEngine/graphics/SpriteBatch.hpp>

#include <RealWorld/items/ItemOnGround.hpp>
#include <RealWorld/world/World.hpp>


class ItemOnGroundManager {
public:
	ItemOnGroundManager(RE::SpriteBatch& spriteBatch, World& world, Hitbox& playerHitbox, Inventory& playerInventory);
	~ItemOnGroundManager();

	void add(ItemOnGround& itemOG);
	//Checks and blocks creation of empty item inside
	void add(const glm::ivec2& pos, const Item& item);

	ulong getNumberOfItemsOG();

	void step();
	void draw();
private:
	RE::SpriteBatch& m_spriteBatch;
	World& m_world;
	Hitbox& m_playerHitbox;
	Inventory& m_playerInventory;
	std::vector<ItemOnGround> m_itemsOG;
	float m_decay = 1.0f;
	float m_defaultLifetime = 4000.0f;

	//Angle deviation
	float m_angleDeviation = 0.0f;//In radians
	float m_angleDeviationMax = 0.9f;//In radians
	float m_angleDeviationChange = 0.06f;
	float m_angleDeviationSign = 1.0f;
};