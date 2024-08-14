/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/simulation/objects/DroppedTilesMgr.hpp>
#include <RealWorld/simulation/objects/shaders/moveDroppedTiles_comp.hpp>

namespace rw {

DroppedTilesMgr::DroppedTilesMgr(const re::PipelineLayout& pipelineLayout)
    : m_moveDroppedTilesPl(
          {.pipelineLayout = *pipelineLayout,
           .debugName      = "rw::DroppedTilesMgr::moveDroppedTilesPl"},
          {.comp = moveDroppedTiles_comp}
      ) {
}

void DroppedTilesMgr::step(const ActionCmdBuf& acb) {
    auto dbg = acb->createDebugRegion("rw::DroppedTilesMgr::step");
    acb.action([&](const re::CommandBuffer& cb) {
        cb->bindPipeline(vk::PipelineBindPoint::eCompute, *m_moveDroppedTilesPl);
        cb->dispatchIndirect(
            *m_tilesBuf, offsetof(DroppedTilesSBHeader, dispatchCommand)
        );
    });
}

/*void DroppedTilesMgr::step(Hitbox playerHitbox) {
    constexpr float k_timeDecr = 1.0f / k_physicsStepsPerSecond;
    m_timeSec -= k_timeDecr;
    glm::vec2 playerCenterPx = playerHitbox.botLeftPx() +
                               playerHitbox.dimsPx() * 0.5f;
    for (size_t i = 0; i < m_tiles.size();) {
        DroppedTile& tile      = m_tiles[i];
        glm::vec2 tileCenterPx = tile.botLeftPx + TilePx * 0.5f;
        if ((tile.lifetimeSec -= k_timeDecr) <= 0.0f) { // If item decayed
            remove(i);
            continue;
        } else if (playerHitbox.overlaps(tileCenterPx)) { // If item can be
collected Item item{tile.id, 1}; m_playerInv.fill(item); if (item.count ==
0) { remove(i); continue;
            }
        } else { // If item is close enough to home
            glm::vec2 toPlayer = playerCenterPx - tileCenterPx;
            float dist         = glm::length(toPlayer);
            if (dist <= 250.0f) {
                float speed = 7.0f - 3.0f * (dist / 250.0f);
                tile.botLeftPx += glm::normalize(toPlayer) * speed;
            }
        }
        ++i; // Increment only if not removed
    }
}

void DroppedTilesMgr::draw(re::SpriteBatch& sb) const {
    constexpr float k_texSizeInv = 1.0f / 256.0f;
    constexpr glm::vec2 k_uvSize{1.0f * k_texSizeInv};
    re::Color col{(glm::abs(glm::fract(m_timeSec * 2.0f) - 0.5f)) * 255.0f,
0, 0, 0}; for (const DroppedTile& dropped : m_tiles) { const re::Texture&
tex = section(dropped.id) == ItemIdSection::Blocks ? m_blockAtlasTex :
m_wallAtlasTex; glm::vec2 uv = glm::vec2{dropped.variant, 255 -
offsetInSection(dropped.id)} * k_texSizeInv; sb.add(tex,
glm::vec4{dropped.botLeftPx, TilePx}, glm::vec4{uv, k_uvSize}, col);
    }
}

void DroppedTilesMgr::remove(size_t i) {
    m_tiles[i] = m_tiles.back();
    m_tiles.pop_back();
}*/

} // namespace rw
