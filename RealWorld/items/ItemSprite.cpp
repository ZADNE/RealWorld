/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemSprite.hpp>

#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>

ItemSprite::ItemSprite() :
    ItemSprite{ItemId::Empty} {
}

ItemSprite::ItemSprite(ItemId id) :
    m_tex(RE::RM::texture({.file = k_atlasPrefix + ItemDatabase::md(id).textureAtlas})),
    m_sprite{
        *m_tex,
        ItemDatabase::md(id).spriteIndex, 0.0f, 1.0f,
        RE::Color{255u, 255u, 255u, 255u},
        glm::vec2(ItemDatabase::md(id).drawScale)
    } {

}

ItemSprite::ItemSprite(ItemSample item) :
    ItemSprite{item.id } {
}

RE::SpriteComplex& ItemSprite::sprite() {
    return m_sprite;
}

