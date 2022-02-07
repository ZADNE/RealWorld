#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world/physics/Health.hpp>
#include <RealWorld/world/physics/DynamicHitbox.hpp>
#include <RealWorld/world/physics/PlayerData.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/ItemInstructionDatabase.hpp>
#include <RealWorld/items/ItemCombinator.hpp>


class Player {
public:
	Player(World& world, RE::SpriteBatch& spriteBatch, ItemOnGroundManager& itemOnGroundManager);
	~Player();

	void adoptPlayerData(const PlayerData& pd);
	void gatherPlayerData(PlayerData& pd) const;

	void jump();
	void walkLeft(bool go);
	void walkRight(bool go);

	//Getters
	glm::vec2 getCenter();
	DynamicHitbox& getHitbox();
	Inventory& getMainInventory();
	ItemUser& getItemUser();
	ItemCombinator& getItemCombinator();
	ItemInstructionDatabase& getIID();

	//Should be called every physics beginStep (not draw beginStep)
	void step();
	void endStep(const glm::ivec2& cursorRel);

	void draw();
private:
	float m_acceleration = 1.0f;
	float m_maxSpeed = 10.0f;
	float m_jumpSpeed = 10.0f;

	float m_walkDirection = 0.0f;

	RE::SpriteBatch& m_spriteBatch;

	Health m_health;
	DynamicHitbox m_hitbox;

	Inventory m_mainInventory;
	ItemUser m_itemUser;
	ItemCombinator m_itemCombinator;
	ItemInstructionDatabase m_instructionDatabase;

	RE::TexturePtr m_playerTex = RE::ResourceManager::getTexture("test");
};