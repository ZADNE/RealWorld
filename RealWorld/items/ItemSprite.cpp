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
    m_tex(RE::RM::texture({.file = ATLAS_PREFIX + ItemDatabase::md(ID).textureAtlas})),
    m_sprite{
        *m_tex,
        ItemDatabase::md(ID).spriteIndex, 0.0f, 1.0f,
        RE::Color{255u, 255u, 255u, 255u},
        glm::vec2(ItemDatabase::md(ID).drawScale)
    } {

}

ItemSprite::ItemSprite(ItemSample item) :
    ItemSprite{item.ID} {
}

RE::SpriteComplex& ItemSprite::sprite() {
    return m_sprite;
}

