#include "ItemOnGround.hpp"

#include <algorithm>

#include <glm\geometric.hpp>

#include <RealEngine/SpriteBatch.hpp>

#include "Inventory.hpp"
#include "../world/TDB.hpp"


ItemOnGround::ItemOnGround(const glm::ivec2& pos, World* world, const Item& item, float lifetime, std::pair<Hitbox*, Inventory*> target) :
	m_tex(RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(item.ID).textureAtlas)),
	m_hitbox(pos, m_tex->getSubimageDims(), world, m_tex->getPivot()),
	m_item(item),
	m_lifetime(lifetime),
	m_target(target) {
	m_hitbox.setFriction(glm::vec2(0.05f, 0.05f));
}

ItemOnGround::~ItemOnGround() {

}

bool ItemOnGround::step(float decay, float angleDeviation) {
	glm::vec2 vec = (glm::vec2)(m_target.first->getPos() - m_hitbox.getPos());

	if (glm::length(vec) < 300.0f) {
		float temp = std::max(4.0f, glm::length(m_hitbox.getVelocity()));

		float velocity = std::max(8.0f - glm::length(vec) / 80.0f, temp);
		velocity = std::max(velocity, m_hitbox.getVelocity().y);

		float radAngle = atan2(vec.y, vec.x) + angleDeviation;

		m_hitbox.setVelocity(glm::vec2(velocity * std::cos(radAngle), velocity * std::sin(radAngle)));

		if (m_hitbox.collidesWith(*m_target.first)) {
			*m_target.second += m_item;
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
	spriteBatch.addSubimage(m_tex.get(), m_hitbox.getPos(), 0, glm::vec2(0.0f, IDB::g(m_item.ID).spriteIndex));
}
