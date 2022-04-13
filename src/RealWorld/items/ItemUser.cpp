#include <RealWorld/items/ItemUser.hpp>


ItemUser::ItemUser(World& world, Inventory& inventory, Hitbox& operatorsHitbox, RE::SpriteBatch& spriteBatch) :
	m_world(world),
	m_inv(inventory),
	m_operatorsHitbox(operatorsHitbox),
	m_spriteBatch(spriteBatch) {

	m_item = &m_inv[m_chosenSlot][0];
}

ItemUser::~ItemUser() {

}

void ItemUser::switchShape() {
	m_shape = m_shape == SET_SHAPE::DISC ? SET_SHAPE::SQUARE : SET_SHAPE::DISC;
}

void ItemUser::resizeShape(float change) {
	m_diameter = glm::clamp(m_diameter + change, 0.5f, 7.5f);
}

void ItemUser::selectSlot(int slot) {
	m_chosenSlot = slot;
	m_item = &m_inv[m_chosenSlot][0];
}

void ItemUser::step(bool use[2], const glm::ivec2& relCursorPosPx) {
	//Update usage
	for (int i = 0; i < 2; i++) {
		if (use[i] != (m_using[i] > 0)) {
			m_using[i] = use[i] ? +1 : -1;
		} else {
			m_using[i] += glm::sign(m_using[i]);
		}
	}

	const ItemMetadata& im = IDB::g(m_item->ID);

	//MAIN
	if (m_using[PRIMARY_USE] > 0) {
		switch (im.type) {
		case ITEM_TYPE::EMPTY:
			break;
		case ITEM_TYPE::PICKAXE:
			m_world.set(SET_TARGET::BLOCK, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(BLOCK::AIR, 0));
			break;
		case ITEM_TYPE::HAMMER:
			m_world.set(SET_TARGET::WALL, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(WALL::AIR, 0));
			break;
		}
	}
	//ALTERNATIVE
	if (m_using[SECONDARY_USE] > 0) {
		switch (im.type) {
		case ITEM_TYPE::EMPTY:
			break;
		case ITEM_TYPE::BLOCK:
			m_world.set(SET_TARGET::BLOCK, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(im.typeIndex, 256));
			break;
		case ITEM_TYPE::WALL:
			m_world.set(SET_TARGET::WALL, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(im.typeIndex, 256));
			break;
		}
	}
}

void ItemUser::draw() {
	/*Item& item = m_inv[m_chosenSlot][0];
	const ItemMetadata& im = IDB::g(item.ID);
	if (m_using[PRIMARY_USE] > 0) {
		if (im.type == ITEM_TYPE::PICKAXE && m_neededToMineBlock != 0.0f && m_UCBlock != BLOCK::AIR) {
			m_spriteBatch.addSubimage(m_miningBlockTex.get(), glm::vec2(m_UCTilePx), 10, glm::vec2(m_neededToMineBlock / TDB::gb(m_UCBlock).toughness * 9.0f, 0.0f));
		}

		if (im.type == ITEM_TYPE::HAMMER && m_neededToMineWall != 0.0f && m_UCWall != WALL::AIR) {
			m_spriteBatch.addSubimage(m_miningBlockTex.get(), glm::vec2(m_UCTilePx), 10, glm::vec2(m_neededToMineWall / TDB::gw(m_UCWall).toughness * 9.0f, 0.0f));
		}
	}*/
}
