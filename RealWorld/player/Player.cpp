/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/player/Player.hpp>

#include <glm/common.hpp>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>
#include <RealEngine/rendering/CommandBuffer.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;


Player::Player():
    m_hitboxBuf(sizeof(PlayerHitboxSB), eStorageBuffer | eTransferDst | eTransferSrc, eDeviceLocal, PlayerHitboxSB{
        .dimsPx = glm::ivec2(m_playerTex.subimageDims()) - glm::ivec2(1),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    }),
    m_hitboxStageBuf(sizeof(PlayerHitboxSB), eTransferDst | eTransferSrc, eHostVisible | eHostCoherent, PlayerHitboxSB{
        .dimsPx = glm::ivec2(m_playerTex.subimageDims()) - glm::ivec2(1),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    }) {
    m_descriptorSet.write(vk::DescriptorType::eStorageBuffer, 1u, 0u, m_hitboxBuf, 0u, sizeof(PlayerHitboxSB));
}

void Player::adoptSave(const PlayerSave& save, const RE::Texture& worldTexture) {
    *m_hitboxStageMapped = PlayerHitboxSB{
        .botLeftPx = save.pos,
        .dimsPx = m_playerTex.subimageDims(),
        .velocityPx = glm::vec2(0.0f, 0.0f)
    };
    m_descriptorSet.write(vk::DescriptorType::eStorageImage, 0u, 0u, worldTexture, vk::ImageLayout::eGeneral);
    RE::CommandBuffer::doOneTimeSubmit([&](const vk::CommandBuffer& commandBuffer) {
        auto copyRegion = vk::BufferCopy2{0ull, 0ull, sizeof(PlayerHitboxSB)};
        commandBuffer.copyBuffer2(vk::CopyBufferInfo2{
            *m_hitboxStageBuf,
            *m_hitboxBuf,
            copyRegion
        });
    });
}

void Player::gatherSave(PlayerSave& save) const {
    save.pos = m_hitboxStageMapped->botLeftPx;
}

glm::vec2 Player::center() const {
    return m_hitboxStageMapped->botLeftPx + m_hitboxStageMapped->dimsPx * 0.5f;
}

void Player::step(const vk::CommandBuffer& commandBuffer, float dir, bool jump, bool autojump) {
    //Simulate the movement
    m_pushConstants.walkDirection = glm::sign(dir);
    m_pushConstants.jump_autojump = glm::vec2(jump, autojump);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_movePlayerPl);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descriptorSet, {});
    commandBuffer.pushConstants<PlayerMovementPC>(*m_pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0u, m_pushConstants);
    commandBuffer.dispatch(1u, 1u, 1u);

    //Copy back the results
    auto copyRegion = vk::BufferCopy2{0ull, 0ull, sizeof(PlayerHitboxSB)};
    auto bufferBarrier = vk::BufferMemoryBarrier2{
        S::eComputeShader,                                                          //Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead,                             //Src access mask
        S::eTransfer,                                                               //Dst stage mask
        A::eTransferRead,                                                           //Dst access mask
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
        *m_hitboxBuf,
        copyRegion.srcOffset,
        copyRegion.size
    };
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, bufferBarrier, {}});
    commandBuffer.copyBuffer2(vk::CopyBufferInfo2{
        *m_hitboxBuf,
        *m_hitboxStageBuf,
        copyRegion
    });
}

void Player::draw(RE::SpriteBatch& spriteBatch) {
    spriteBatch.add(m_playerTex, glm::vec4{m_hitboxStageMapped->botLeftPx, m_hitboxStageMapped->dimsPx}, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f});
}
