#include <RealWorld/items/ItemOnGround.hpp>

#include <algorithm>

#include <glm/geometric.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>

#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/world/TDB.hpp>


ItemOnGround::ItemOnGround(const glm::ivec2& pos, World& world, const Item& item, float lifetime, Hitbox& targetHitbox, Inventory& targetInventory) :
	m_tex(RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(item.ID).textureAtlas)),
	m_hitbox(&world, pos, m_tex->getSubimageDims(), m_tex->getPivot()),
	m_item(item),
	m_lifetime(lifetime),
	m_targetHitbox(&targetHitbox),
	m_targetInventory(&targetInventory) {
	m_hitbox.setFriction(glm::vec2(0.05f, 0.05f));
}

ItemOnGround::~ItemOnGround() {

}

bool ItemOnGround::step(float decay, float angleDeviation) {
	glm::vec2 vec = (glm::vec2)(m_targetHitbox->getCenter() - m_hitbox.getCenter());

	if (glm::length(vec) < 300.0f) {
		float temp = std::max(4.0f, glm::length(m_hitbox.getVelocity()));

		float velocity = std::max(8.0f - glm::length(vec) / 80.0f, temp);
		velocity = std::max(velocity, m_hitbox.getVelocity().y);

		float radAngle = atan2(vec.y, vec.x) + angleDeviation;

		m_hitbox.velocity() = glm::vec2(velocity * std::cos(radAngle), velocity * std::sin(radAngle));

		if (m_hitbox.collidesWith(*m_targetHitbox)) {
			*m_targetInventory += m_item;
			if (m_item.isEmpty()) {
				return true;//Item is empty
			}
		}
	}

	m_hitbox.step();
	m_lifetime -= decay;
	if (m_lifetime <= 0.0f) {
		return true;//Item decayed
	}

	return false;
}

void ItemOnGround::draw(RE::SpriteBatch& spriteBatch) const {
	spriteBatch.addSubimage(m_tex.get(), m_hitbox.getCenter(), 0, glm::vec2(0.0f, IDB::g(m_item.ID).spriteIndex));
}
