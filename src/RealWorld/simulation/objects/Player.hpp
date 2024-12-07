/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/batches/SpriteBatch.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>

#include <RealWorld/save/WorldSave.hpp>
#include <RealWorld/simulation/general/ActionCmdBuf.hpp>
#include <RealWorld/simulation/objects/Hitbox.hpp>
#include <RealWorld/simulation/objects/shaders/AllShaders.hpp>
#include <RealWorld/simulation/objects/shaders/PlayerHitboxSB_glsl.hpp>
#include <RealWorld/simulation/objects/shaders/PlayerMovementPC_glsl.hpp>

namespace rw {

/**
 * @brief Simulates and draws the user-controlled character
 */
class Player {
public:
    Player()
        : Player(re::TextureShaped{re::TextureSeed{"player"}}) {}

    void adoptSave(
        const PlayerSave& save, const re::Texture& worldTexture, glm::ivec2 worldTexCh
    );
    void gatherSave(PlayerSave& save) const;

    glm::vec2 centerPx() const;

    Hitbox hitbox() const {
        return Hitbox{botLeftPx(), m_hitboxStageBuf->dimsPx};
    }

    /**
     * @brief Moves the player based on its surroundings tiles and user input
     * @param acb Command buffer that will be used to record the commands
     * @param dir Walking direction: -1 = go left, 0 = stay still, +1 = go right
     * @param jump The player jumps if this is true and if the player stands on
     * solid ground
     * @param autojump Tells whether the player should automatically jump
     * over obstacles
     */
    void step(const ActionCmdBuf& acb, float dir, bool jump, bool autojump);

    void draw(re::SpriteBatch& spriteBatch);

private:

    glsl::PlayerMovementPC m_pc{
        .acceleration    = 0.5f,
        .maxWalkVelocity = 6.0f,
        .jumpVelocity    = 7.0f,
        .writeIndex      = 1
    };

    Player(re::TextureShaped&& playerTex)
        : Player(
              std::move(playerTex),
              glsl::PlayerHitboxSB{
                  .dimsPx = glm::ivec2(playerTex.subimageDims()) - 1,
              }
          ) {}

    Player(re::TextureShaped&& playerTex, const glsl::PlayerHitboxSB& initSb);

    re::TextureShaped m_playerTex;

    re::Buffer m_hitboxBuf;
    re::BufferMapped<glsl::PlayerHitboxSB> m_hitboxStageBuf;
    glm::vec2 m_oldBotLeftPx{};

    re::PipelineLayout m_pipelineLayout{{}, {.comp = glsl::movePlayer_comp}};
    re::Pipeline m_movePlayerPl{
        {.pipelineLayout = *m_pipelineLayout, .debugName = "rw::Player::movePlayer"},
        {.comp = glsl::movePlayer_comp}
    };
    re::DescriptorSet m_descriptorSet{re::DescriptorSetCreateInfo{
        .layout    = m_pipelineLayout.descriptorSetLayout(0),
        .debugName = "rw::Player::descriptorSet"
    }};

    glm::vec2 botLeftPx() const;
};

} // namespace rw
