/*!
 *  @author    Dubsky Tomas
 */
#include <glm/common.hpp>

#include <RealEngine/graphics/batches/SpriteBatch.hpp>
#include <RealEngine/graphics/commands/CommandBuffer.hpp>
#include <RealEngine/graphics/synchronization/DoubleBuffered.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/simulation/objects/Player.hpp>

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;

using D = vk::DescriptorType;
using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

namespace rw {

constexpr glm::uint k_worldTexBinding = 0;
constexpr glm::uint k_playerBinding   = 1;

Player::Player(re::TextureShaped&& playerTex, const glsl::PlayerHitboxSB& initSb)
    : m_playerTex(std::move(playerTex))
    , m_hitboxBuf(re::BufferCreateInfo{
          .memoryUsage = vma::MemoryUsage::eAutoPreferDevice,
          .sizeInBytes = sizeof(glsl::PlayerHitboxSB),
          .usage       = eStorageBuffer | eTransferDst | eTransferSrc,
          .initData    = re::objectToByteSpan(initSb),
          .debugName   = "rw::Player::hitbox"
      })
    , m_hitboxStageBuf(re::BufferCreateInfo{
          .allocFlags = vma::AllocationCreateFlagBits::eMapped |
                        vma::AllocationCreateFlagBits::eHostAccessRandom,
          .sizeInBytes = sizeof(glsl::PlayerHitboxSB),
          .usage       = eTransferDst | eTransferSrc,
          .initData    = re::objectToByteSpan(initSb),
          .debugName   = "rw::Player::hitboxStage"
      }) {
    m_descriptorSet.write(D::eStorageBuffer, k_playerBinding, 0u, m_hitboxBuf);
}

void Player::adoptSave(
    const PlayerSave& save, const re::Texture& worldTexture, glm::ivec2 worldTexCh
) {
    m_pc.worldTexMaskTi = chToTi(worldTexCh) - 1;

    m_oldBotLeftPx    = save.pos;
    *m_hitboxStageBuf = glsl::PlayerHitboxSB{
        .botLeftPx  = {save.pos, save.pos},
        .dimsPx     = m_playerTex.subimageDims() - 1.0f,
        .velocityPx = glm::vec2(0.0f, 0.0f)
    };
    m_descriptorSet.write(
        D::eStorageImage, k_worldTexBinding, 0u, worldTexture, vk::ImageLayout::eGeneral
    );
    re::CommandBuffer::doOneTimeSubmit([&](const re::CommandBuffer& cb) {
        vk::BufferCopy2 copyRegion{0ull, 0ull, sizeof(glsl::PlayerHitboxSB)};
        cb->copyBuffer2(vk::CopyBufferInfo2{
            m_hitboxStageBuf.buffer(), *m_hitboxBuf, copyRegion
        });
    });
}

void Player::gatherSave(PlayerSave& save) const {
    save.pos = botLeftPx();
}

glm::vec2 Player::centerPx() const {
    return botLeftPx() + m_hitboxStageBuf->dimsPx * 0.5f;
}

void Player::step(const ActionCmdBuf& acb, float dir, bool jump, bool autojump) {
    auto dbg = acb->createDebugRegion("player");

    // Store position from previous step
    auto readIndex = re::StepDoubleBufferingState::readIndex();
    m_oldBotLeftPx = m_hitboxStageBuf->botLeftPx[readIndex];

    // Copy back results of previous step
    size_t bufOffset = offsetof(glsl::PlayerHitboxSB, botLeftPx[0]) +
                       sizeof(glsl::PlayerHitboxSB::botLeftPx[0]) * readIndex;
    auto copyRegion = vk::BufferCopy2{bufOffset, bufOffset, sizeof(glm::vec2)};
    (*acb)->copyBuffer2(
        vk::CopyBufferInfo2{*m_hitboxBuf, m_hitboxStageBuf.buffer(), copyRegion}
    );

    // Simulate the movement
    acb.action(
        [&](const re::CommandBuffer& cb) {
            m_pc.writeIndex    = re::StepDoubleBufferingState::writeIndex();
            m_pc.walkDirection = glm::sign(dir);
            m_pc.jump          = jump;
            m_pc.autojump      = autojump;
            cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_movePlayerPl);
            cb->bindDescriptorSets(
                vk::PipelineBindPoint::eCompute, *m_pipelineLayout, 0u,
                *m_descriptorSet, {}
            );
            cb->pushConstants<glsl::PlayerMovementPC>(
                *m_pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0u, m_pc
            );
            cb->dispatch(1u, 1u, 1u);
        },
        ImageAccess{
            .name   = ImageTrackName::World,
            .stage  = S::eComputeShader,
            .access = A::eShaderStorageRead,
            .layout = vk::ImageLayout::eGeneral
        }
    );
}

void Player::draw(re::SpriteBatch& spriteBatch) {
    spriteBatch.add(
        m_playerTex, glm::vec4{botLeftPx(), m_hitboxStageBuf->dimsPx},
        glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}
    );
}

glm::vec2 Player::botLeftPx() const {
    return m_oldBotLeftPx;
}

} // namespace rw
