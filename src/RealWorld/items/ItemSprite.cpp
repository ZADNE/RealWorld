#include <RealWorld/items/ItemSprite.hpp>

#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>

ItemSprite::ItemSprite() :
	FullSprite{ RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(ITEM::EMPTY).textureAtlas), IDB::g(ITEM::EMPTY).spriteIndex, 0.0f, 1.0f, RE::Color{255u, 255u, 255u, 255u}, glm::vec2(IDB::g(ITEM::EMPTY).drawScale) } {

}

ItemSprite::ItemSprite(ITEM ID) :
	FullSprite{ RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(ID).textureAtlas), IDB::g(ID).spriteIndex, 0.0f, 1.0f, RE::Color{255u, 255u, 255u, 255u}, glm::vec2(IDB::g(ID).drawScale) } {

}

ItemSprite::ItemSprite(Item item) :
	FullSprite{ RE::RM::getTexture(ITEM_ATLAS_PREFIX + IDB::g(item.ID).textureAtlas), IDB::g(item.ID).spriteIndex, 0.0f, 1.0f, RE::Color{255u, 255u, 255u, 255u}, glm::vec2(IDB::g(item.ID).drawScale) } {

}
