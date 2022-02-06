#include <RealWorld/world/physics/Player.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>

#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/ItemCombinator.hpp>

Player::Player(const RE::InputManager& inputManager, World& world, RE::SpriteBatch& spriteBatch, ItemOnGroundManager& itemOnGroundManager) :
	m_health(100.0f),
	m_hitbox({0, 0}, glm::ivec2(28, 40), glm::ivec2(0, 0)),
	m_mainInventory(glm::ivec2(10, 5)),
	m_spriteBatch(spriteBatch),
	m_inputManager(inputManager),
	m_itemUser(world, m_mainInventory, m_hitbox, spriteBatch, itemOnGroundManager) {

	m_hitbox.setFriction(glm::vec2(0.8f, 0.0f));
	m_hitbox.init(&world);

	m_itemCombinator.connectToInventory(&m_mainInventory);
	m_itemCombinator.connectToIID(&m_instructionDatabase);

}

Player::~Player() {

}

void Player::adoptPlayerData(const PlayerData& pd) {
	m_hitbox.setPosition(pd.pos);
	m_mainInventory.adoptInventoryData(pd.id);
}

void Player::gatherPlayerData(PlayerData& pd) const {
	pd.pos = m_hitbox.getPos();
	m_mainInventory.gatherInventoryData(pd.id);
}

glm::ivec2 Player::getPos() {
	return m_hitbox.getPos();
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
	//Jumping
	bool grounded = m_hitbox.isGrounded();
	if (m_inputManager.wasPressed(KB(PLAYER_JUMP)) && grounded) {
		float multiplier = m_inputManager.isDown(RE::Key::LShift) ? 2.0f : 1.0f;
		m_hitbox.setVelocityY(m_jumpSpeed * multiplier);
	}
	//Moving left
	if (m_inputManager.isDown(KB(PLAYER_LEFT))) {
		m_hitbox.addVelocityX(m_acceleration * -1.0f);
	}
	//Moving right
	if (m_inputManager.isDown(KB(PLAYER_RIGHT))) {
		m_hitbox.addVelocityX(m_acceleration);
	}
	//Limiting speed
	m_hitbox.limitVelocityX(m_maxSpeed);

	m_hitbox.step();

	//Item combinator
	m_itemCombinator.step();
}

void Player::endStep(const glm::ivec2& cursorRel) {
	//Item user
	m_itemUser.step(cursorRel);
}

void Player::draw() {
	m_spriteBatch.addTexture(m_playerTex.get(), glm::vec2{m_hitbox.getPos()}, 0);
	m_itemUser.draw();
}
