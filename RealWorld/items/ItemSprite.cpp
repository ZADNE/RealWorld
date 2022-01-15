#include "ItemSprite.hpp"

#include <RealEngine/ResourceManager.hpp>

#include "Item.hpp"

ItemSprite::ItemSprite() :
	FullSprite{ RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(I_ID::EMPTY).textureAtlas), IDB::g(I_ID::EMPTY).spriteIndex, 0.0f, 1.0f, RE::Colour{255u, 255u, 255u, 255u}, glm::vec2(IDB::g(I_ID::EMPTY).drawScale) } {

}

ItemSprite::ItemSprite(I_ID ID):
	FullSprite{ RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(ID).textureAtlas), IDB::g(ID).spriteIndex, 0.0f, 1.0f, RE::Colour{255u, 255u, 255u, 255u}, glm::vec2(IDB::g(ID).drawScale) } {

}

ItemSprite::ItemSprite(Item item) :
	FullSprite{ RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(item.ID).textureAtlas), IDB::g(item.ID).spriteIndex, 0.0f, 1.0f, RE::Colour{255u, 255u, 255u, 255u}, glm::vec2(IDB::g(item.ID).drawScale) } {

}
