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
	if (m_hitbox.overlapsBlocks({0, -iTILE_SIZE.y})) {
		m_hitbox.velocity().y = m_jumpSpeed;
	}
}

void Player::walk(WALK dir) {
	m_walkDirection = dir;
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

void Player::step(bool autojump) {
	auto& vel = m_hitbox.velocity();

	vel.x += static_cast<float>(m_walkDirection) * m_acceleration;

	//Friction
	if (m_walkDirection == WALK::STAY) {
		vel.x -= glm::sign(vel.x);
	}
	vel.x = glm::clamp(vel.x, -m_maxSpeed, m_maxSpeed);

	if (autojump && m_walkDirection != WALK::STAY && m_hitbox.overlapsBlocks({vel.x * 4, 0})) {
		jump();//Autojump
	}

	m_hitbox.step();

	m_itemCombinator.step();
}

void Player::endStep(const glm::ivec2& cursorRel) {
	m_itemUser.step(cursorRel);
}

void Player::draw() {
	m_spriteBatch.addTexture(m_playerTex.get(), m_hitbox.getBotLeft(), 0);
	m_itemUser.draw();
}
