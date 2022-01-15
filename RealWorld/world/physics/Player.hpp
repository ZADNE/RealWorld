#pragma once
#include <glm\glm.hpp>

#include <RealEngine/ResourceManager.hpp>
#include <RealEngine/InputManager.hpp>

#include "Health.hpp"
#include "DynamicHitbox.hpp"
#include "../../items/Inventory.hpp"
#include "../../KeyBinder.hpp"
#include "../../items/ItemInstructionDatabase.hpp"
#include "PlayerData.hpp"

class World;
class ItemUser;
class ItemCombinator;
class ItemOnGroundManager;
namespace RE {
class SpriteBatch;
}

class Player {
public:
	Player();
	~Player();

	void init(RE::InputManager* inputmanager, World* world, RE::SpriteBatch* spriteBatch, ItemOnGroundManager* itemOnGroundManager);

	void adoptPlayerData(const PlayerData& pd);
	void gatherPlayerData(PlayerData& pd);

	//Movement (should be called before step)
	//TO BE DONE (jump(), goLeft(), goRight())
	//It is now done inside step() instead.

	//Getters
	glm::ivec2 getPos();
	DynamicHitbox& getHitbox();
	Inventory* getMainInventory();
	ItemUser* getItemUser();
	ItemCombinator* getItemCombinator();
	ItemInstructionDatabase& getIID();

	//Should be called every physics step (not draw step)
	void beginStep();
	void endStep(const glm::ivec2& cursorRel);

	void draw();
private:
	float m_acceleration = 1.0f;
	float m_maxSpeed = 10.0f;
	float m_jumpSpeed = 9.0f;

	RE::SpriteBatch* m_spriteBatch = nullptr;

	Health m_health;
	DynamicHitbox m_hitbox;
	RE::InputManager* m_inputManager = nullptr;

	Inventory* m_mainInventory = nullptr;
	ItemUser* m_itemUser = nullptr;
	ItemCombinator* m_itemCombinator = nullptr;
	ItemInstructionDatabase m_instructionDatabase;

	RE::TexturePtr m_playerTex = RE::ResourceManager::getTexture("test");
};