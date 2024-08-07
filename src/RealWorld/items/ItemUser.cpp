﻿/*!
 *  @author    Dubsky Tomas
 */
#include <glm/ext/scalar_constants.hpp>

#include <RealWorld/items/ItemUser.hpp>

namespace rw {

ItemUser::ItemUser(World& world, Inventory& inventory)
    : m_world(world)
    , m_inv(inventory) {
    m_item = &m_inv[m_chosenSlot][0];
}

void ItemUser::switchShape() {
    using enum World::ModificationShape;
    m_shape = m_shape == Disk ? Square : Disk;
}

void ItemUser::resizeShape(float change) {
    m_radiusTi = glm::clamp(m_radiusTi + change, 0.5f, 7.5f);
}

void ItemUser::selectSlot(int slot) {
    m_chosenSlot = slot;
    m_item       = &m_inv[m_chosenSlot][0];
}

void ItemUser::step(
    const ActionCmdBuf& acb, bool usePrimary, bool useSecondary, glm::ivec2 relCursorPosPx
) {
    auto dbg    = acb->createDebugRegion("item user");
    bool use[2] = {usePrimary, useSecondary};

    // Update usage
    for (int i = 0; i < 2; i++) {
        if (use[i] != (m_using[i] > 0)) {
            m_using[i] = use[i] ? +1 : -1;
        } else {
            m_using[i] += glm::sign(m_using[i]);
        }
    }

    const ItemMetadata& md = ItemDatabase::md(m_item->id);

    // Main
    if (m_using[k_primaryUse] > 0) {
        switch (md.type) {
        case ItemType::Empty: break;
        case ItemType::Pickaxe:
            m_world.modify(
                acb, TileLayer::Block, m_shape, m_radiusTi,
                pxToTi(relCursorPosPx), glm::uvec2(Block::Remove, 0)
            );
            break;
        case ItemType::Hammer:
            m_world.modify(
                acb, TileLayer::Wall, m_shape, m_radiusTi,
                pxToTi(relCursorPosPx), glm::uvec2(Wall::Remove, 0)
            );
            break;
        }
    }
    // Alternative
    if (m_using[k_secondaryUse] > 0) {
        switch (md.type) {
        case ItemType::Empty: break;
        case ItemType::Block:
            m_world.modify(
                acb, TileLayer::Block, m_shape, m_radiusTi,
                pxToTi(relCursorPosPx), glm::uvec2(md.typeIndex, 256)
            );
            break;
        case ItemType::Wall:
            m_world.modify(
                acb, TileLayer::Wall, m_shape, m_radiusTi,
                pxToTi(relCursorPosPx), glm::uvec2(md.typeIndex, 256)
            );
            break;
        }
    }
}

void ItemUser::render(glm::vec2 relCursorPosPx, re::GeometryBatch& gb) {
    const ItemMetadata& md = ItemDatabase::md(m_item->id);

    if (md.type != ItemType::Empty) {
        re::Color col{255, 255, 255, 255};
        glm::vec2 center = tiToPx(pxToTi(relCursorPosPx)) + TilePx * 0.5f;
        float radPx      = m_radiusTi * TilePx.x;
        if (m_shape == World::ModificationShape::Disk) {
            constexpr float k_quality   = 16.0f;
            constexpr float k_angleIncr = glm::pi<float>() * 2.0f / k_quality;
            std::array<re::VertexPoCo, static_cast<size_t>(k_quality * 2.0f)> vertices;
            for (float i = 0.0f; i < k_quality; i++) {
                float a     = i * k_angleIncr;
                glm::vec2 p = center + glm::vec2{cos(a), sin(a)} * radPx;
                vertices[static_cast<size_t>(i * 2.0f) % vertices.size()] =
                    re::VertexPoCo{p, col};
                vertices[static_cast<size_t>(i * 2.0f - 1.0f) % vertices.size()] =
                    re::VertexPoCo{p, col};
            }
            gb.addVertices(vertices);
        } else if (m_shape == World::ModificationShape::Square) {
            auto tl = center + glm::vec2{-radPx, +radPx};
            auto tr = center + glm::vec2{+radPx, +radPx};
            auto br = center + glm::vec2{+radPx, -radPx};
            auto bl = center + glm::vec2{-radPx, -radPx};
            std::array<re::VertexPoCo, 8> vertices = std::to_array<re::VertexPoCo>({
                {tl, col},
                {tr, col},
                {tr, col},
                {br, col},
                {br, col},
                {bl, col},
                {bl, col},
                {tl, col},
            });
            gb.addVertices(vertices);
        }
    }
}

} // namespace rw
