#include <RealWorld/world/physics/Player.hpp>

#include <RealEngine/SpriteBatch.hpp>

#include <RealWorld/items/ItemUser.hpp>
#include <RealWorld/items/ItemCombinator.hpp>

Player::Player() :
	m_health(100.0f),
	m_hitbox({0, 0}, glm::ivec2(28, 40), glm::ivec2(0, 0))
{
	m_hitbox.setFriction(glm::vec2(0.8f, 0.0f));
	//Inventory
	m_mainInventory = new Inventory{glm::ivec2(10, 5)};
}

Player::~Player() {
	delete m_itemUser;
	delete m_itemCombinator;
	delete m_mainInventory;
}

void Player::init(RE::InputManager* inputmanager, World* world, RE::SpriteBatch* spriteBatch, ItemOnGroundManager* itemOnGroundManager) {
	m_inputManager = inputmanager;
	m_hitbox.init(world);
	m_spriteBatch = spriteBatch;
	//Item user
	m_itemUser = new ItemUser{};
	m_itemUser->init(world, m_mainInventory, &m_hitbox, spriteBatch, itemOnGroundManager);
	//Item combinator
	m_itemCombinator = new ItemCombinator{};
	m_itemCombinator->connectToInventory(m_mainInventory);
	m_itemCombinator->connectToIID(&m_instructionDatabase);
}

void Player::adoptPlayerData(const PlayerData& pd) {
	m_hitbox.setPosition(pd.pos);
	m_mainInventory->adoptInventoryData(pd.id);
}

void Player::gatherPlayerData(PlayerData& pd) {
	pd.pos = m_hitbox.getPos();
	m_mainInventory->gatherInventoryData(pd.id);
}

glm::ivec2 Player::getPos() {
	return m_hitbox.getPos();
}

DynamicHitbox& Player::getHitbox() {
	return m_hitbox;
}

Inventory* Player::getMainInventory() {
	return m_mainInventory;
}

ItemUser* Player::getItemUser() {
	return m_itemUser;
}

ItemCombinator* Player::getItemCombinator() {
	return m_itemCombinator;
}

ItemInstructionDatabase& Player::getIID() {
	return m_instructionDatabase;
}

void Player::beginStep() {
	//Jumping
	bool grounded = m_hitbox.isGrounded();
	if (m_inputManager->wasPressed(KB(PLAYER_JUMP)) && grounded) {
		m_hitbox.setVelocityY(m_jumpSpeed);
	}
	//Moving left
	if (m_inputManager->isDown(KB(PLAYER_LEFT))) {
		m_hitbox.addVelocityX(m_acceleration * -1.0f);
	}
	//Moving right
	if (m_inputManager->isDown(KB(PLAYER_RIGHT))) {
		m_hitbox.addVelocityX(m_acceleration);
	}
	//Limiting speed
	m_hitbox.limitVelocityX(m_maxSpeed);

	m_hitbox.step();

	//Item combinator
	m_itemCombinator->step();
}

void Player::endStep(const glm::ivec2& cursorRel) {
	//Item user
	m_itemUser->step(cursorRel);
}

void Player::draw() {
	m_spriteBatch->addTexture(m_playerTex.get(), glm::vec2{m_hitbox.getPos()}, 0);
	m_itemUser->draw();
}
