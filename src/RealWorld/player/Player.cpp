/*!
 *  @author    Dubsky Tomas
 */
#include <glm/common.hpp>

#include <RealEngine/graphics/CommandBuffer.hpp>
#include <RealEngine/graphics/batches/SpriteBatch.hpp>

#include <RealWorld/player/Player.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

Player::Player(re::TextureShaped&& playerTex, const PlayerHitboxSB& initSb)
    : m_playerTex(std::move(playerTex))
    , m_hitboxBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(PlayerHitboxSB),
          .usage       = eStorageBuffer | eTransferDst | eTransferSrc,
          .initData    = re::objectToByteSpan(initSb)})
    , m_hitboxStageBuf(re::BufferCreateInfo{
          .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                        vma::AllocationCreateFlagBits::eHostAccessRandom,
          .sizeInBytes = sizeof(PlayerHitboxSB),
          .usage       = eTransferDst | eTransferSrc,
          .initData    = re::objectToByteSpan(initSb)}) {
    m_descriptorSet.write(
        vk::DescriptorType::eStorageBuffer, 1u, 0u, m_hitboxBuf, 0u, sizeof(PlayerHitboxSB)
    );
}

void Player::adoptSave(const PlayerSave& save, const re::Texture& worldTexture) {
    *m_hitboxStageBuf = PlayerHitboxSB{
        .botLeftPx  = {save.pos, save.pos},
        .dimsPx     = m_playerTex.subimageDims(),
        .velocityPx = glm::vec2(0.0f, 0.0f)};
    m_descriptorSet.write(
        vk::DescriptorType::eStorageImage, 0u, 0u, worldTexture, vk::ImageLayout::eGeneral
    );
    re::CommandBuffer::doOneTimeSubmit([&](const vk::CommandBuffer& cmdBuf) {
        auto copyRegion = vk::BufferCopy2{0ull, 0ull, sizeof(PlayerHitboxSB)};
        cmdBuf.copyBuffer2(vk::CopyBufferInfo2{
            m_hitboxStageBuf.buffer(), *m_hitboxBuf, copyRegion});
    });
}

void Player::gatherSave(PlayerSave& save) const {
    save.pos = botLeftPx();
}

glm::vec2 Player::center() const {
    return botLeftPx() + m_hitboxStageBuf->dimsPx * 0.5f;
}

void Player::step(const vk::CommandBuffer& cmdBuf, float dir, bool jump, bool autojump) {
    // Simulate the movement
    m_pushConstants.writeIndex    = 1 - m_pushConstants.writeIndex;
    m_pushConstants.walkDirection = glm::sign(dir);
    m_pushConstants.jump          = jump;
    m_pushConstants.autojump      = autojump;
    cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, *m_movePlayerPl);
    cmdBuf.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u, *m_descriptorSet, {}
    );
    cmdBuf.pushConstants<PlayerMovementPC>(
        *m_pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0u, m_pushConstants
    );
    cmdBuf.dispatch(1u, 1u, 1u);

    // Copy back the results
    size_t writeOffset = offsetof(PlayerHitboxSB, botLeftPx[0]) +
                         sizeof(PlayerHitboxSB::botLeftPx[0]) *
                             m_pushConstants.writeIndex;
    auto copyRegion = vk::BufferCopy2{writeOffset, writeOffset, sizeof(glm::vec2)};
    auto bufferBarrier = vk::BufferMemoryBarrier2{
        S::eComputeShader,                              // Src stage mask
        A::eShaderStorageWrite | A::eShaderStorageRead, // Src access mask
        S::eTransfer,                                   // Dst stage mask
        A::eTransferRead,                               // Dst access mask
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        *m_hitboxBuf,
        copyRegion.srcOffset,
        copyRegion.size};
    cmdBuf.pipelineBarrier2(vk::DependencyInfo{{}, {}, bufferBarrier, {}});
    cmdBuf.copyBuffer2(vk::CopyBufferInfo2{
        *m_hitboxBuf, m_hitboxStageBuf.buffer(), copyRegion});
}

void Player::draw(re::SpriteBatch& spriteBatch) {
    spriteBatch.add(
        m_playerTex,
        glm::vec4{botLeftPx(), m_hitboxStageBuf->dimsPx},
        glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}
    );
}

const glm::vec2& Player::botLeftPx() const {
    return m_hitboxStageBuf->botLeftPx[1 - m_pushConstants.writeIndex];
}

} // namespace rw
