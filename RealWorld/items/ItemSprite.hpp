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
template<RE::Renderer R>
class ItemSprite {
public:

    ItemSprite();
    ItemSprite(ITEM ID);
    ItemSprite(ItemSample item);

    RE::SpriteComplex<R>& sprite();

private:

    RE::SharedTexture m_tex;
    RE::SpriteComplex<R> m_sprite;

    /**
     * @brief Filenames of all item atlases must begin with this prefix
    */
    static inline constexpr std::string ATLAS_PREFIX = "itemAtlas";
};
