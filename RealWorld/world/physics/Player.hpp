#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/user_input/InputManager.hpp>

#include <RealWorld/world/physics/Health.hpp>
#include <RealWorld/world/physics/DynamicHitbox.hpp>
#include <RealWorld/world/physics/PlayerData.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/ItemInstructionDatabase.hpp>
#include <RealWorld/items/ItemCombinator.hpp>
#include <RealWorld/KeyBinder.hpp>


class Player {
public:
	Player(const RE::InputManager& inputManager, World& world, FurnitureManager& furnitureManager, RE::SpriteBatch& spriteBatch, ItemOnGroundManager& itemOnGroundManager);
	~Player();

	void adoptPlayerData(const PlayerData& pd);
	void gatherPlayerData(PlayerData& pd) const;

	//Movement (should be called before beginStep)
	//TO BE DONE (jump(), goLeft(), goRight())
	//It is now done inside beginStep() instead.

	//Getters
	glm::ivec2 getPos();
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
	float m_jumpSpeed = 17.0f;

	RE::SpriteBatch& m_spriteBatch;

	Health m_health;
	DynamicHitbox m_hitbox;
	const RE::InputManager& m_inputManager;

	Inventory m_mainInventory;
	ItemUser m_itemUser;
	ItemCombinator m_itemCombinator;
	ItemInstructionDatabase m_instructionDatabase;

	RE::TexturePtr m_playerTex = RE::ResourceManager::getTexture("test");
};