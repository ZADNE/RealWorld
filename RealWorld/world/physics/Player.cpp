#include <RealWorld/world/physics/Player.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>

#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/ItemCombinator.hpp>

Player::Player(World& world, RE::SpriteBatch& spriteBatch, ItemOnGroundManager& itemOnGroundManager) :
	m_health(100.0f),
	m_hitbox(&world, {0, 0}, {28, 40}, {14, 20}),
	m_mainInventory(glm::ivec2(10, 5)),
	m_spriteBatch(spriteBatch),
	m_itemUser(world, m_mainInventory, m_hitbox, spriteBatch, itemOnGroundManager) {

	m_hitbox.setFriction(glm::vec2(0.8f, 0.0f));

	m_itemCombinator.connectToInventory(&m_mainInventory);
	m_itemCombinator.connectToIID(&m_instructionDatabase);

}

Player::~Player() {

}

void Player::adoptPlayerData(const PlayerData& pd) {
	m_hitbox.botLeft() = pd.pos;
	m_mainInventory.adoptInventoryData(pd.id);
}

void Player::gatherPlayerData(PlayerData& pd) const {
	pd.pos = m_hitbox.getBotLeft();
	m_mainInventory.gatherInventoryData(pd.id);
}

void Player::jump() {
	if (m_hitbox.isGrounded()) {
		m_hitbox.velocity().y = m_jumpSpeed;
	}
}

void Player::walkLeft(bool go) {
	m_walkDirection += go ? -1.0f : +1.0f;
}

void Player::walkRight(bool go) {
	m_walkDirection += go ? 1.0f : -1.0f;
}

glm::vec2 Player::getCenter() {
	return m_hitbox.getCenter();
}

DynamicHitbox& Player::getHitbox() {
	return m_hitbox;
}

Inventory& Player::getMainInventory() {
	return m_mainInventory;
}

ItemUser& Player::getItemUser() {
	return m_itemUser;
}

ItemCombinator& Player::getItemCombinator() {
	return m_itemCombinator;
}

ItemInstructionDatabase& Player::getIID() {
	return m_instructionDatabase;
}

void Player::step() {
	m_hitbox.velocity().x += m_walkDirection * m_acceleration;
	m_hitbox.velocity().x = glm::clamp(m_hitbox.velocity().x, -m_maxSpeed, m_maxSpeed);

	if (m_hitbox.overlapsBlocks({m_hitbox.velocity().x, 0.0f})) {
		for (float autoJumpBLocks = 1.0f; autoJumpBLocks <= 4.0f; autoJumpBLocks++) {
			if (!m_hitbox.overlapsBlocks({m_hitbox.velocity().x, autoJumpBLocks * TILE_SIZE.y})) {
				m_hitbox.botLeft().y += autoJumpBLocks * TILE_SIZE.y;//Autojump
				break;
			}
		}

	}

	m_hitbox.step();

	m_itemCombinator.step();
}

void Player::endStep(const glm::ivec2& cursorRel) {
	m_itemUser.step(cursorRel);
}

void Player::draw() {
	m_spriteBatch.addTexture(m_playerTex.get(), glm::floor(m_hitbox.getBotLeft()), 0);
	m_itemUser.draw();
}
