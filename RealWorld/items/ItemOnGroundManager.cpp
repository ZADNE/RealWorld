#include <RealWorld/items/ItemOnGroundManager.hpp>

#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/IDB.hpp>


ItemOnGroundManager::ItemOnGroundManager() {

}

ItemOnGroundManager::~ItemOnGroundManager() {

}

void ItemOnGroundManager::init(RE::SpriteBatch* spriteBatch, World* world, std::pair<Hitbox*, Inventory*> defaultTarget) {
	m_spriteBatch = spriteBatch;
	m_world = world;
	m_defaultTarget = defaultTarget;
}

void ItemOnGroundManager::add(ItemOnGround& itemOG) {
	m_itemsOG.push_back(itemOG);
}

void ItemOnGroundManager::add(const glm::ivec2& pos, const Item& item) {
	if (item.isEmpty()) { return; }
	m_itemsOG.emplace_back(pos, m_world, item, m_defaultLifetime, m_defaultTarget);
}

ulong ItemOnGroundManager::getNumberOfItemsOG() {
	return (ulong)m_itemsOG.size();
}

void ItemOnGroundManager::step() {
	m_angleDeviation += m_angleDeviationChange * m_angleDeviationSign;
	if (m_angleDeviation > m_angleDeviationMax || m_angleDeviation < -m_angleDeviationMax) { m_angleDeviationSign *= -1; }

	for (size_t i = 0; i < m_itemsOG.size(); ) {
		if (m_itemsOG[i].step(m_decay, m_angleDeviation)) {
			m_itemsOG[i] = m_itemsOG[m_itemsOG.size() - 1];
			m_itemsOG.pop_back();
		} else { i++; }
	}
}

void ItemOnGroundManager::draw() {
	for (const auto& itemOG : m_itemsOG) {
		itemOG.draw(*m_spriteBatch);
	}
}