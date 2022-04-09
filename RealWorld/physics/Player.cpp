#include <RealWorld/physics/Player.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>

#include <RealWorld/items/ItemUser.hpp>

Player::Player(RE::SpriteBatch& spriteBatch) :
	m_hitbox({0, 0}, {28, 40}, {14, 20}),
	m_spriteBatch(spriteBatch) {
	m_movementUBO.connectToShaderProgram(m_playerDynamicsShader, 0u);

	m_hitboxSSBO.connectToShaderProgram(m_playerDynamicsShader, 0u);
}

Player::~Player() {

}

void Player::adoptPlayerData(const PlayerData& pd) {
	m_hitbox.botLeft() = pd.pos;
	m_hitboxSSBO.overwrite(offsetof(PlayerHitboxSSBO, botLeftPx), glm::vec2(pd.pos));
}

void Player::gatherPlayerData(PlayerData& pd) const {
	pd.pos = m_hitbox.getBotLeft();
}

Hitbox& Player::getHitbox() {
	return m_hitbox;
}

void Player::step(WALK dir, bool jump, bool autojump) {
	const auto* hitboxSSBO = m_hitboxSSBO.map<PlayerHitboxSSBO>(offsetof(PlayerHitboxSSBO, botLeftPx), sizeof(PlayerHitboxSSBO), READ);
	m_hitbox.botLeft() = hitboxSSBO->botLeftPx;
	m_hitboxSSBO.unmap();

	PlayerMovementUBO movement{
		.walkDirection = glm::sign(static_cast<float>(dir)),
		.jump_autojump = glm::vec2(jump, autojump)
	};
	m_movementUBO.overwrite(offsetof(PlayerMovementUBO, walkDirection), sizeof(float) + sizeof(glm::vec2), &movement.walkDirection);
	m_playerDynamicsShader.dispatchCompute({1, 1, 1}, true);
}

void Player::draw() {
	m_spriteBatch.addTexture(m_playerTex.get(), m_hitbox.getBotLeft(), 0);
}
