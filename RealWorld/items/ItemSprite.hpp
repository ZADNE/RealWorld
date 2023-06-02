/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/rendering/batches/Sprite.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>

 /**
  * @brief Allows convenient drawing of items in inventory
 */
class ItemSprite {
public:

    ItemSprite();
    ItemSprite(ItemId id);
    ItemSprite(ItemSample item);

    re::SpriteComplex& sprite();

private:

    re::SharedTexture m_tex;
    re::SpriteComplex m_sprite;

    /**
     * @brief Filenames of all item atlases must begin with this prefix
    */
    static inline const std::string k_atlasPrefix = "itemAtlas";
};
