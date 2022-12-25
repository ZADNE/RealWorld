﻿/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/player/Player.hpp>

#include <glm/common.hpp>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>

#include <RealWorld/reserved_units/images.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;

Player::Player(const RE::Texture& worldTexture) :
    m_hitboxBuf(sizeof(PlayerHitboxSB), eStorageBuffer | eTransferDst | eTransferSrc, eDeviceLocal, PlayerHitboxSB{
        .dimsPx = glm::ivec2(m_playerTex.subimageDims()) - glm::ivec2(1),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    }),
    m_hitboxStageBuf(sizeof(PlayerHitboxSB), eTransferDst | eTransferSrc, eHostVisible | eHostCoherent, PlayerHitboxSB{
        .dimsPx = glm::ivec2(m_playerTex.subimageDims()) - glm::ivec2(1),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    }) {
    m_descriptorSet.write(vk::DescriptorType::eStorageImage, 0u, 0u, worldTexture);
    m_descriptorSet.write(vk::DescriptorType::eStorageBuffer, 1u, m_hitboxBuf, 0u, sizeof(PlayerHitboxSB));
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
    commandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_movePlayerPl.pipeline());
    commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_movePlayerPl.pipelineLayout(), 0u, m_descriptorSet.descriptorSet(), {});
    m_pushConstants.walkDirection = glm::sign(static_cast<float>(dir));
    m_pushConstants.jump_autojump = glm::vec2(jump, autojump);
    commandBuffer->pushConstants<MovementPushConstants>(m_movePlayerPl.pipelineLayout(), vk::ShaderStageFlagBits::eCompute, 0u, m_pushConstants);
    commandBuffer->dispatch(1u, 1u, 1u);
}

void Player::draw(RE::SpriteBatch& spriteBatch) {
    spriteBatch.add(m_playerTex, glm::vec4{m_hitboxStageMapped->botLeftPx, m_hitboxStageMapped->dimsPx}, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f});
}
