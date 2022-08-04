/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/ShaderProgram.hpp>
#include <RealEngine/graphics/SpriteBatch.hpp>

#include <RealWorld/physics/Hitbox.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/shaders/simulation.hpp>
#include <RealWorld/save/WorldSave.hpp>

enum class WALK : int {
	LEFT = -1,
	STAY = 0,
	RIGHT = 1
};

/**
 * @brief Represents the user-controlled character.
*/
class Player {
public:
	Player(RE::SpriteBatch& spriteBatch);

	void adoptSave(const PlayerSave& save);
	void gatherSave(PlayerSave& save) const;

	Hitbox& getHitbox();

	void step(WALK dir, bool jump, bool autojump);

	void draw();
private:
	using enum RE::BufferUsageFlags;
	using enum RE::BufferMapUsageFlags;

	RE::SpriteBatch& m_spriteBatch;

	Hitbox m_hitbox;

	RE::TexturePtr m_playerTex = RE::ResourceManager::getTexture("player");

	struct PlayerMovementUBO {
		float acceleration;
		float maxWalkVelocity;
		float jumpVelocity;
		float walkDirection;
		glm::vec2 jump_autojump;
	};
	RE::TypedBuffer m_movementBuf{UNIF_BUF_PLAYERMOVEMENT, DYNAMIC_STORAGE, PlayerMovementUBO{
		.acceleration = 0.5f,
		.maxWalkVelocity = 6.0f,
		.jumpVelocity = 7.0f
	}};

	struct PlayerHitboxSSBO {
		glm::vec2 botLeftPx;
		glm::vec2 dimsPx;
		glm::vec2 velocityPx;
	};
	RE::TypedBuffer m_hitboxBuf{STRG_BUF_PLAYER, DYNAMIC_STORAGE | MAP_READ, PlayerHitboxSSBO{
		.dimsPx = glm::ivec2(m_playerTex->getTrueDims()) - glm::ivec2(1),
		.velocityPx = glm::vec2(0.0f, 0.0f)
	}};

	RE::ShaderProgram m_playerMovementShd{{.comp = playerMovement_comp}};
};