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
    explicit ItemSprite(Item item)
        : ItemSprite{item.id} {}

    const re::SpriteComplex& sprite() const { return m_sprite; }
    re::SpriteComplex& sprite() { return m_sprite; }

private:
    re::SharedTextureShaped m_tex;
    re::SpriteComplex m_sprite;

    /**
     * @brief Filenames of all item atlases must begin with this prefix
     */
    static inline const std::string k_atlasPrefix = "itemAtlas";
};

} // namespace rw
