/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/batches/Sprite.hpp>
#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>

namespace rw {

/**
 * @brief Allows convenient drawing of items in inventory
 */
class ItemSprite {
public:
    ItemSprite();
    explicit ItemSprite(ItemId id);
    explicit ItemSprite(ItemSample item);

    re::SpriteComplex& sprite();

private:
    re::SharedTexture m_tex;
    re::SpriteComplex m_sprite;

    /**
     * @brief Filenames of all item atlases must begin with this prefix
     */
    static inline const std::string k_atlasPrefix = "itemAtlas";
};

} // namespace rw
