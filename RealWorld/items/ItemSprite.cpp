/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemSprite.hpp>

#include <RealEngine/resources/ResourceManager.hpp>

#include <RealWorld/items/Item.hpp>

template<RE::Renderer R>
ItemSprite<R>::ItemSprite() :
    ItemSprite<R>{ITEM::EMPTY} {
}

template<RE::Renderer R>
ItemSprite<R>::ItemSprite(ITEM ID) :
    m_tex(RE::RM::texture(ATLAS_PREFIX + ItemDatabase::md(ID).textureAtlas)),
    m_sprite{
        m_tex->get<R>(),
        ItemDatabase::md(ID).spriteIndex, 0.0f, 1.0f,
        RE::Color{255u, 255u, 255u, 255u},
        glm::vec2(ItemDatabase::md(ID).drawScale)
    } {

}

template<RE::Renderer R>
ItemSprite<R>::ItemSprite(ItemSample item) :
    ItemSprite<R>{item.ID} {
}

template<RE::Renderer R>
RE::SpriteComplex<R>& ItemSprite<R>::sprite() {
    return m_sprite;
}

template ItemSprite<RE::RendererGL46>;
