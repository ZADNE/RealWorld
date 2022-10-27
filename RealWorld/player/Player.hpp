/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/rendering/vertices/ShaderProgram.hpp>
#include <RealEngine/rendering/batches/SpriteBatch.hpp>

#include <RealWorld/player/Hitbox.hpp>
#include <RealWorld/reserved_units/buffers.hpp>
#include <RealWorld/player/shaders/AllShaders.hpp>
#include <RealWorld/save/WorldSave.hpp>

enum class WALK : int {
    LEFT = -1,
    STAY = 0,
    RIGHT = 1
};

/**
 * @brief Represents the user-controlled character.
*/
template<RE::Renderer R>
class Player {
public:

    Player(RE::SpriteBatch<R>& spriteBatch);

    void adoptSave(const PlayerSave& save);
    void gatherSave(PlayerSave& save) const;

    Hitbox& getHitbox();

    void step(WALK dir, bool jump, bool autojump);

    void draw();

private:

    using enum RE::BufferUsageFlags;
    using enum RE::BufferMapUsageFlags;

    RE::SpriteBatch<R>& m_sb;

    Hitbox m_hitbox;

    RE::Texture<R> m_playerTex{{.file = "player"}};

    struct PlayerMovementUBO {
        float acceleration;
        float maxWalkVelocity;
        float jumpVelocity;
        float walkDirection;
        glm::vec2 jump_autojump;
    };
    RE::BufferTyped<R> m_movementBuf{ UNIF_BUF_PLAYERMOVEMENT, DYNAMIC_STORAGE, PlayerMovementUBO{
        .acceleration = 0.5f,
        .maxWalkVelocity = 6.0f,
        .jumpVelocity = 7.0f
    } };

    struct PlayerHitboxSSBO {
        glm::vec2 botLeftPx;
        glm::vec2 dimsPx;
        glm::vec2 velocityPx;
    };
    RE::BufferTyped<R> m_hitboxBuf{ STRG_BUF_PLAYER, DYNAMIC_STORAGE | MAP_READ, PlayerHitboxSSBO{
        .dimsPx = glm::ivec2(m_playerTex.getTrueDims()) - glm::ivec2(1),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    } };

    RE::ShaderProgram<R> m_movePlayerShd{ {.comp = movePlayer_comp} };
};
