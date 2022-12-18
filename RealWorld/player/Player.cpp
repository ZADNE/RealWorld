/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/player/Player.hpp>

#include <glm/common.hpp>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;


Player::Player(RE::SpriteBatch& spriteBatch) :
    m_sb(spriteBatch),
    m_hitboxBuf(sizeof(PlayerHitboxSSBO), eStorageBuffer | eTransferDst | eTransferSrc, eDeviceLocal, PlayerHitboxSSBO{
        .dimsPx = glm::ivec2(m_playerTex.getTrueDims()) - glm::ivec2(1),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    }),
    m_hitboxStageBuf(sizeof(PlayerHitboxSSBO), eTransferDst | eTransferSrc, eHostVisible | eHostCoherent, PlayerHitboxSSBO{
        .dimsPx = glm::ivec2(m_playerTex.getTrueDims()) - glm::ivec2(1),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    }) {
    //m_movePlayerShd.backInterfaceBlock(0u, UNIF_BUF_PLAYERMOVEMENT);
    //m_movePlayerShd.backInterfaceBlock(0u, STRG_BUF_PLAYER);
}

void Player::adoptSave(const PlayerSave& save) {
    m_hitboxStageMapped->botLeftPx = save.pos;
}

void Player::gatherSave(PlayerSave& save) const {
    save.pos = m_hitboxStageMapped->botLeftPx;
}

glm::vec2 Player::getCenter() const {
    return m_hitboxStageMapped->botLeftPx + m_hitboxStageMapped->dimsPx * 0.5f;
}

void Player::step(RE::CommandBuffer& commandBuffer, WALK dir, bool jump, bool autojump) {
    m_pushConstants.walkDirection = glm::sign(static_cast<float>(dir));
    m_pushConstants.jump_autojump = glm::vec2(jump, autojump);
    m_movePlayerPl.bind(vk::PipelineBindPoint::eCompute);
    commandBuffer->pushConstants<MovementPushConstants>(m_movePlayerPl.pipelineLayout(), vk::ShaderStageFlagBits::eCompute, 0u, m_pushConstants);
    commandBuffer->dispatch(1u, 1u, 1u);
}

void Player::draw() {
    //m_sb.addTexture(m_playerTex, m_hitbox.getBotLeft(), 0);
}
