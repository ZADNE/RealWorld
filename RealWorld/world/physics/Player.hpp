#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/world/physics/Health.hpp>
#include <RealWorld/world/physics/DynamicHitbox.hpp>
#include <RealWorld/world/physics/PlayerData.hpp>

enum class WALK : int {
	LEFT = -1,
	STAY = 0,
	RIGHT = 1
};

class Player {
public:
	Player(World& world, RE::SpriteBatch& spriteBatch);
	~Player();

	void adoptPlayerData(const PlayerData& pd);
	void gatherPlayerData(PlayerData& pd) const;

	void jump();
	void walk(WALK dir);

	DynamicHitbox& getHitbox();

	void step(bool autojump);

	void draw();
private:
	float m_acceleration = 0.5f;
	float m_maxSpeed = 6.0f;
	float m_jumpSpeed = 7.0f;

	WALK m_walkDirection = WALK::STAY;

	RE::SpriteBatch& m_spriteBatch;

	Health m_health;
	DynamicHitbox m_hitbox;

	RE::TexturePtr m_playerTex = RE::ResourceManager::getTexture("test");
};