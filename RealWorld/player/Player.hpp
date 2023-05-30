/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>

#include <RealWorld/player/shaders/AllShaders.hpp>
#include <RealWorld/save/WorldSave.hpp>

/**
 * @brief Simulates and draws the user-controlled character
 */
class Player {
public:
    Player();

    void adoptSave(const PlayerSave& save, const RE::Texture& worldTexture);
    void gatherSave(PlayerSave& save) const;

    glm::vec2 center() const;

    /**
     * @brief Moves the player based on its surroundings tiles and user input
     * @param dir Walking direction: -1 = go left, 0 = stay still, +1 = go right
     * @param jump The player jumps if this is true and if the player stands on
     * solid ground
     * @param autojump Tells whether the player should automatically jump
     * over obstacles
     */
    void step(
        const vk::CommandBuffer& commandBuffer,
        float                    dir,
        bool                     jump,
        bool                     autojump
    );

    void draw(RE::SpriteBatch& spriteBatch);

private:
    RE::TextureShaped m_playerTex{{.file = "player"}};

    struct PlayerMovementPC {
        float acceleration;
        float maxWalkVelocity;
        float jumpVelocity;
        float walkDirection;
        float jump;
        float autojump;
        int writeIndex; // Selects PlayerHitboxSB::botLeftPx, swings every step
    };
    PlayerMovementPC m_pushConstants{
        .acceleration    = 0.5f,
        .maxWalkVelocity = 6.0f,
        .jumpVelocity    = 7.0f,
        .writeIndex      = 1};

    struct PlayerHitboxSB {
        glm::vec2 botLeftPx[2];
        glm::vec2 dimsPx;
        glm::vec2 velocityPx;
    };
    RE::Buffer      m_hitboxBuf;
    RE::Buffer      m_hitboxStageBuf;
    PlayerHitboxSB* m_hitboxStageMapped =
        m_hitboxStageBuf.map<PlayerHitboxSB>(0u, sizeof(PlayerHitboxSB));

    RE::PipelineLayout m_pipelineLayout{{}, {.comp = movePlayer_comp}};
    RE::Pipeline       m_movePlayerPl{
              {.pipelineLayout = *m_pipelineLayout}, {.comp = movePlayer_comp}};
    RE::DescriptorSet m_descriptorSet{m_pipelineLayout, 0u};

    const glm::vec2& botLeftPx() const;
};
