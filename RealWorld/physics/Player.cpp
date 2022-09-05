/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/physics/Player.hpp>

#include <RealEngine/rendering/batches/SpriteBatch.hpp>

#include <RealWorld/items/ItemUser.hpp>


template<RE::Renderer R>
Player<R>::Player(RE::SpriteBatch<R>& spriteBatch) :
    m_hitbox({ 0, 0 }, { 28, 40 }, { 14, 20 }),
    m_spriteBatch(spriteBatch) {
    m_playerMovementShd.backInterfaceBlock(0u, UNIF_BUF_PLAYERMOVEMENT);
    m_playerMovementShd.backInterfaceBlock(0u, STRG_BUF_PLAYER);
}

template<RE::Renderer R>
void Player<R>::adoptSave(const PlayerSave& save) {
    m_hitbox.botLeft() = save.pos;
    m_hitboxBuf.overwrite(offsetof(PlayerHitboxSSBO, botLeftPx), glm::vec2(save.pos));
}

template<RE::Renderer R>
void Player<R>::gatherSave(PlayerSave& save) const {
    save.pos = m_hitbox.getBotLeft();
}

template<RE::Renderer R>
Hitbox& Player<R>::getHitbox() {
    return m_hitbox;
}

template<RE::Renderer R>
void Player<R>::step(WALK dir, bool jump, bool autojump) {
    const auto* hitboxSSBO = m_hitboxBuf.map<PlayerHitboxSSBO>(offsetof(PlayerHitboxSSBO, botLeftPx), sizeof(PlayerHitboxSSBO), READ);
    m_hitbox.botLeft() = hitboxSSBO->botLeftPx;
    m_hitboxBuf.unmap();

    PlayerMovementUBO movement{
        .walkDirection = glm::sign(static_cast<float>(dir)),
        .jump_autojump = glm::vec2(jump, autojump)
    };
    m_movementBuf.overwrite(offsetof(PlayerMovementUBO, walkDirection), sizeof(float) + sizeof(glm::vec2), &movement.walkDirection);
    m_playerMovementShd.dispatchCompute({ 1, 1, 1 }, true);
}

template<RE::Renderer R>
void Player<R>::draw() {
    m_spriteBatch.addTexture(m_playerTex, m_hitbox.getBotLeft(), 0);
}

template Player<RE::RendererGL46>;
