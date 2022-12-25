/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/rendering/CommandBuffer.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>
#include <RealEngine/rendering/batches/SpriteBatch.hpp>

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
class Player {
public:

    Player(const RE::Texture& worldTexture);

    void adoptSave(const PlayerSave& save);
    void gatherSave(PlayerSave& save) const;

    glm::vec2 getCenter() const;

    void step(RE::CommandBuffer& commandBuffer, WALK dir, bool jump, bool autojump);

    void draw(RE::SpriteBatch& spriteBatch);

private:

    RE::TextureShaped m_playerTex{{.file = "player"}};

    struct MovementPushConstants {
        float acceleration;
        float maxWalkVelocity;
        float jumpVelocity;
        float walkDirection;
        glm::vec2 jump_autojump;
    };
    MovementPushConstants m_pushConstants{
        .acceleration = 0.5f,
        .maxWalkVelocity = 6.0f,
        .jumpVelocity = 7.0f
    };

    struct PlayerHitboxSB {
        glm::vec2 botLeftPx;
        glm::vec2 dimsPx;
        glm::vec2 velocityPx;
    };
    RE::Buffer m_hitboxBuf;
    RE::Buffer m_hitboxStageBuf;
    PlayerHitboxSB* m_hitboxStageMapped = m_hitboxStageBuf.map<PlayerHitboxSB>(0u, sizeof(PlayerHitboxSB));

    RE::Pipeline m_movePlayerPl{movePlayer_comp};
    RE::DescriptorSet m_descriptorSet{m_movePlayerPl};
};
