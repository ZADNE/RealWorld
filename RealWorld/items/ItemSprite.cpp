/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemSprite.hpp>

#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>

ItemSprite::ItemSprite() :
	ItemSprite{ITEM::EMPTY} {

}

ItemSprite::ItemSprite(ITEM ID) :
	FullSprite{
		RE::RM::getTexture(ATLAS_PREFIX + ItemDatabase::md(ID).textureAtlas),
		ItemDatabase::md(ID).spriteIndex, 0.0f, 1.0f,
		RE::Color{255u, 255u, 255u, 255u},
		glm::vec2(ItemDatabase::md(ID).drawScale)
	} {

}

ItemSprite::ItemSprite(ItemSample item) :
	ItemSprite{item.ID} {

}
