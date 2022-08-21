/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/batches/Sprite.hpp>

#include <RealWorld/items/Item.hpp>

 /**
  * @brief Allows convenient drawing of items in inventory
 */
class ItemSprite : public RE::FullSprite {
public:
	ItemSprite();
	ItemSprite(ITEM ID);
	ItemSprite(ItemSample item);

private:
	/**
	 * Filenames of all item atlases must begin with this prefix
	*/
	static inline const std::string ATLAS_PREFIX = "itemAtlas";
};
