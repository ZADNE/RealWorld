#include <RealWorld/items/ItemUser.hpp>

#include <RealEngine/graphics/SpriteBatch.hpp>
#include <RealEngine/utility/utility.hpp>

#include <RealWorld/world/TDB.hpp>
#include <RealWorld/items/IDB.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/world/World.hpp>
#include <RealWorld/items/ItemOnGroundManager.hpp>
#include <RealWorld/world/physics/position_conversions.hpp>


ItemUser::ItemUser() {

}

ItemUser::~ItemUser() {

}

void ItemUser::init(World* world, Inventory* inventory, Hitbox* operatorsHitbox, RE::SpriteBatch* spriteBatch, ItemOnGroundManager* itemOnGroundManager) {
	m_world = world;
	m_inv = inventory;
	m_operatorsHitbox = operatorsHitbox;
	m_spriteBatch = spriteBatch;
	m_itemOnGroundManager = itemOnGroundManager;
	m_item = &(*m_inv)[m_chosenSlot][0];
}

void ItemUser::beginUse(ItemUse use) {
	if (use == ItemUse::NUMBER_OF_USES) { return; }
	m_using[use] = 1;
}

void ItemUser::endUse(ItemUse use) {
	if (use == ItemUse::NUMBER_OF_USES) { return; }
	m_using[use] = 0;
	if (use == ItemUse::MAIN) {
		m_neededToMineBlock = 0.0f;
		m_neededToMineWall = 0.0f;
	}
}

void ItemUser::chooseSlot(int slot) {
	if (m_chosenSlot != slot) {//If actually changed slot
		m_neededToMineBlock = 0.0f;
		m_neededToMineWall = 0.0f;
	}
	m_chosenSlot = slot;
	m_item = &(*m_inv)[m_chosenSlot][0];
	if (IDB::g(m_item->ID).type == I_TYPE::FURNITURE) {
		checkBuildFurniture();
	}
}

void ItemUser::step(const glm::ivec2& relCursorPos) {
	m_relCursorPos = relCursorPos;
	reloadTarget();

	if (m_inv == nullptr) { return; }
	ItemMetadata im = IDB::g(m_item->ID);

	//MAIN
	if (m_using[ItemUse::MAIN] > 0) {
		switch (im.type) {
		case I_TYPE::EMPTY:
			break;
		case I_TYPE::PICKAXE:
			//Dismantling furniture
			if (m_using[ItemUse::MAIN] == 1 && //If just clicked
				rmath::distance(m_operatorsHitbox->getPos(), m_UCTilePx) <= pickaxeMetadata[im.typeIndex].range) {//If the operator stands close enough
				auto fur = m_world->destroy(m_UCTileBc);
				m_itemOnGroundManager->add(fur.second * ivec2_BLOCK_SIZE, Item(FDB::getItemID(fur.first), 1));
			}

			//Mining blocks
			if (m_UCTileBc == m_UCTileBcP) {//Still mining same tile
				if (pickaxeMetadata[im.typeIndex].strength >= TDB::gb(m_UCBlock).hardness//If the pickaxe is strong enough to mine it
					&& rmath::distance(m_operatorsHitbox->getPos(), m_UCTilePx) <= pickaxeMetadata[im.typeIndex].range) {//If the operator stands close enough
					m_neededToMineBlock += pickaxeMetadata[im.typeIndex].speed;
					if (m_neededToMineBlock >= TDB::gb(m_UCBlock).toughness) {//Finished mining
						m_world->set(chunk::SET_TYPES::BLOCK, m_UCTileBc, (uchar)BLOCK_ID::AIR);
						m_itemOnGroundManager->add(glm::ivec2(m_UCTilePx - ivec2_BLOCK_SIZE / 2), Item(TDB::gb(m_UCBlock).itemID, 1));
						m_UCBlock = BLOCK_ID::AIR;
					}
				} else {//Moved away from the block
					m_neededToMineBlock = 0.0f;
				}
			} else {//Moved cursor, not mining the same block
				m_neededToMineBlock = 0.0f;
			}
			break;
		case I_TYPE::HAMMER:
			if (m_UCTileBc == m_UCTileBcP) {//Still mining same tile
				if (m_UCBlock == BLOCK_ID::AIR) {//If there is air in front of the wall
					if (hammerMetadata[im.typeIndex].strength >= TDB::gw(m_UCWall).hardness//If the hammer is strong enough to mine it
						&& rmath::distance(m_operatorsHitbox->getPos(), m_UCTilePx) <= hammerMetadata[im.typeIndex].range) {//If the operator stands close enough
						m_neededToMineWall += hammerMetadata[im.typeIndex].speed;
						if (m_neededToMineWall >= TDB::gw(m_UCWall).toughness) {//Finished mining
							m_world->set(chunk::SET_TYPES::WALL, m_UCTileBc, (uchar)WALL_ID::AIR);
							m_itemOnGroundManager->add(glm::ivec2(m_UCTilePx - ivec2_BLOCK_SIZE / 2), Item(TDB::gw(m_UCWall).itemID, 1));
							m_UCWall = WALL_ID::AIR;
						}
					} else {//Moved away from the wall
						m_neededToMineWall = 0.0f;
					}
				} else {
					m_neededToMineWall = 0.0f;
				}
			} else {//Moved cursor, not mining the same wall
				m_neededToMineWall = 0.0f;
			}
			break;
		}
	}
	//ALTERNATIVE
	if (m_using[ItemUse::ALTERNATIVE] > 0) {
		switch (im.type) {
		case I_TYPE::EMPTY:
			break;
		case I_TYPE::BLOCK:
			if (m_UCBlock == BLOCK_ID::AIR//If there already is not a block
				&& rmath::distance(m_operatorsHitbox->getPos(), m_UCTilePx) <= m_buildingRange//If the operator stands close enough
				&& !m_world->build(FStatic{m_UCTileBc, 0u}, false).isError()) {//If there is not any furniture
				if (!m_operatorsHitbox->overlapsBlockwise(m_relCursorPos)) {//If not inside operator
					m_world->set(chunk::SET_TYPES::BLOCK, m_UCTileBc, im.typeIndex);
					m_UCBlock = (BLOCK_ID)im.typeIndex;
					--(*m_item);
					m_inv->wasChanged();
				}
			}
			break;
		case I_TYPE::WALL:
			if (m_UCWall == WALL_ID::AIR//If there already is not a wall
				&& m_UCBlock == BLOCK_ID::AIR//If there is air on top of the wall
				&& rmath::distance(m_operatorsHitbox->getPos(), m_UCTilePx) <= m_buildingRange) {//If the operator stands close enough
				m_world->set(chunk::SET_TYPES::WALL, m_UCTileBc, im.typeIndex);
				m_UCWall = (WALL_ID)im.typeIndex;
				--(*m_item);
				m_inv->wasChanged();
			}
			break;
		case I_TYPE::FURNITURE:
			if (rmath::distance(m_operatorsHitbox->getPos(), m_UCTilePx + (FDB::getDims((size_t)im.typeIndex) - glm::ivec2{1, 1}) * ivec2_BLOCK_SIZE / 2) <= m_buildingRange) {//If the operator stands close enough
				FIndex findex = m_world->build(FStatic{m_UCTileBc, (size_t)im.typeIndex});//Try to build
				if (findex.getError() == BuildError::NO_ERROR) {//Successfully built
					--(*m_item);
					m_inv->wasChanged();
				}
			}
			break;
		}
	}

	//Incrementing timers
	for (auto& timer : m_using) {
		if (timer > 0u) {
			timer++;
		} else {
			timer--;
		}
	}
}

void ItemUser::draw() {
	Item& item = (*m_inv)[m_chosenSlot][0];
	const ItemMetadata& im = IDB::g(item.ID);
	if (m_using[ItemUse::MAIN] > 0) {
		if (im.type == I_TYPE::PICKAXE && m_neededToMineBlock != 0.0f && m_UCBlock != BLOCK_ID::AIR) {
			m_spriteBatch->addSubimage(m_miningBlockTex.get(), glm::vec2(m_UCTilePx), 10, glm::vec2(m_neededToMineBlock / TDB::gb(m_UCBlock).toughness * 9.0f, 0.0f));
		}

		if (im.type == I_TYPE::HAMMER && m_neededToMineWall != 0.0f && m_UCWall != WALL_ID::AIR) {
			m_spriteBatch->addSubimage(m_miningBlockTex.get(), glm::vec2(m_UCTilePx), 10, glm::vec2(m_neededToMineWall / TDB::gw(m_UCWall).toughness * 9.0f, 0.0f));
		}
	}
	if (im.type == I_TYPE::FURNITURE && m_canBuildFurniture) {
		m_spriteBatch->addSubimage(FDB::getTexture(im.typeIndex).get(), glm::vec2(m_UCTilePx - ivec2_BLOCK_SIZE / 2), 10, m_furBlueprint[m_canBuildFurniture], glm::vec2(0.0f, FDB::getSprite(im.typeIndex)));
	}
}

void ItemUser::checkBuildFurniture() {
	ItemMetadata im = IDB::g(m_item->ID);
	if (rmath::distance(m_operatorsHitbox->getPos(), m_UCTilePx + (FDB::getDims((size_t)im.typeIndex) - glm::ivec2{1, 1}) * ivec2_BLOCK_SIZE / 2) <= m_buildingRange) {//If the operator stands close enough
		FIndex findex = m_world->build(FStatic{m_UCTileBc, (size_t)im.typeIndex}, false);//Try to build
		if (findex.getError() == BuildError::NO_ERROR) {
			m_canBuildFurniture = 2;//Can build
		} else {
			m_canBuildFurniture = 1;//Bad placement/occupied
		}
	} else {
		m_canBuildFurniture = 0;//Too far
	}
}

void ItemUser::reloadTarget() {
	m_UCTileBcP = m_UCTileBc;
	m_UCTilePxP = m_UCTilePx;
	m_UCBlockP = m_UCBlock;
	m_UCWallP = m_UCWall;

	m_UCTileBc = pxToBc(m_relCursorPos);
	m_UCTilePx = bcToPx(m_UCTileBc) + ivec2_BLOCK_SIZE / 2;
	m_UCBlock = (BLOCK_ID)m_world->get(chunk::BLOCK_VALUES::BLOCK, m_UCTileBc);
	m_UCWall = (WALL_ID)m_world->get(chunk::BLOCK_VALUES::WALL, m_UCTileBc);

	if (m_UCTileBc != m_UCTileBcP && IDB::g(m_item->ID).type == I_TYPE::FURNITURE) {
		checkBuildFurniture();
	}
}
