/**
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
    explicit ItemSprite(ItemID id);
    explicit ItemSprite(Item item)
        : ItemSprite{item.id} {}

    const re::SpriteComplex& sprite() const { return m_sprite; }
    re::SpriteComplex& sprite() { return m_sprite; }

private:
    std::shared_ptr<re::TextureShaped> m_tex;
    re::SpriteComplex m_sprite;
};

} // namespace rw
