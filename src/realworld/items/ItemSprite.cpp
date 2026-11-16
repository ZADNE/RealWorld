/**
 *  @author    Dubsky Tomas
 */
module realworld:items.ItemSprite_impl;
import :items.ItemSprite;
import realengine;
import :items.Item;

namespace rw {

ItemSprite::ItemSprite()
    : ItemSprite{ItemID::Empty} {
}

ItemSprite::ItemSprite(ItemID id)
    : m_tex(re::RM::texture(textureID(id)))
    , m_sprite{
          *m_tex,
          spriteIndex(id),
          subimageIndex(id),
          1.0f,
          re::Color{255u, 255u, 255u, 255u},
          glm::vec2{drawScale(id)}
      } {
}

} // namespace rw
