/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/batches/SpriteBatch.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>

#include <RealWorld/player/shaders/AllShaders.hpp>
#include <RealWorld/save/WorldSave.hpp>

namespace rw {

/**
 * @brief Simulates and draws the user-controlled character
 */
class Player {
public:
    Player()
        : Player(re::TextureShaped{re::TextureSeed{"player"}}) {}

    void adoptSave(const PlayerSave& save, const re::Texture& worldTexture);
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
    void step(const vk::CommandBuffer& cmdBuf, float dir, bool jump, bool autojump);

    void draw(re::SpriteBatch& spriteBatch);

private:
    struct PlayerHitboxSB {
        glm::vec2 botLeftPx[2];
        glm::vec2 dimsPx;
        glm::vec2 velocityPx;
    };

    Player(re::TextureShaped&& playerTex)
        : Player(
              std::move(playerTex),
              PlayerHitboxSB{
                  .dimsPx = glm::ivec2(playerTex.subimageDims()) - glm::ivec2(1),
                  .velocityPx = glm::vec2(0.0f, 0.0f)}
          ) {}

    Player(re::TextureShaped&& playerTex, const PlayerHitboxSB& initSb);

    re::TextureShaped m_playerTex;

    struct PlayerMovementPC {
        float acceleration    = 0.5f;
        float maxWalkVelocity = 6.0f;
        float jumpVelocity    = 7.0f;
        float walkDirection;
        float jump;
        float autojump;
        int writeIndex = 1; // Selects PlayerHitboxSB::botLeftPx, swings every step
    } m_pushConstants;

    re::Buffer                       m_hitboxBuf;
    re::BufferMapped<PlayerHitboxSB> m_hitboxStageBuf;
    glm::vec2                        m_oldBotLeftPx{};

    re::PipelineLayout m_pipelineLayout{{}, {.comp = movePlayer_comp}};
    re::Pipeline       m_movePlayerPl{
              {.pipelineLayout = *m_pipelineLayout}, {.comp = movePlayer_comp}};
    re::DescriptorSet m_descriptorSet{m_pipelineLayout.descriptorSetLayout(0)};

    const glm::vec2& botLeftPx() const;
};

} // namespace rw
