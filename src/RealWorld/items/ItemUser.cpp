﻿/**
 *  @author    Dubsky Tomas
 */
#include <array>

#include <glm/ext/scalar_constants.hpp>

#include <RealWorld/items/ItemUser.hpp>

namespace rw {

constexpr float k_modifyTilesGroupSize = 16.0f;
constexpr float k_maxShapeSize         = (k_modifyTilesGroupSize - 1.0f) / 2.0f;

ItemUser::ItemUser(World& world, Inventory& inventory)
    : m_world(world)
    , m_inv(inventory) {
}

void ItemUser::switchShape() {
    using enum World::ModificationShape;
    m_shape = m_shape == Disk ? Square : Disk;
}

void ItemUser::resizeShape(float change) {
    m_radiusTi = glm::clamp(m_radiusTi + change, 0.5f, k_maxShapeSize);
}

void ItemUser::finishSpecRemoval(int count) {
    m_inv[m_selSlot->slotIndex][0] -= count;
}

void ItemUser::step(
    const ActionCmdBuf& acb, int selSlot, bool usePrimary, bool useSecondary,
    glm::ivec2 relCursorPosPx
) {
    auto dbg                = acb->createDebugRegion("item user");
    *m_selSlot              = SelSlotState{selSlot, 0};
    std::array<bool, 2> use = {usePrimary, useSecondary};

    // Update usage
    for (int i = 0; i < 2; i++) {
        if (use[i] != (m_using[i] > 0)) {
            m_using[i] = use[i] ? +1 : -1;
        } else {
            m_using[i] += glm::sign(m_using[i]);
        }
    }

    const Item& item  = selItem();
    ItemIDSection sec = section(item.id);

    if (m_using[k_primaryUse] > 0) { // Main
        switch (sec) {
        case ItemIDSection::Pickaxes:
        case ItemIDSection::Hammers:  {
            auto layer = sec == ItemIDSection::Pickaxes ? TileLayer::Block
                                                        : TileLayer::Wall;
            m_world.mineTiles(acb, layer, m_shape, m_radiusTi, pxToTi(relCursorPosPx));
            break;
        }
        default: break;
        }
    }

    if (m_using[k_secondaryUse] > 0) { // Alternative
        switch (sec) {
        case ItemIDSection::Blocks:
        case ItemIDSection::Walls:  {
            auto layer    = sec == ItemIDSection::Blocks ? TileLayer::Block
                                                         : TileLayer::Wall;
            int specCount = selItemSpecCount();
            if (specCount > 0) {
                m_world.placeTiles(
                    acb, layer, m_shape, m_radiusTi, pxToTi(relCursorPosPx),
                    glm::uvec2{offsetInSection(item.id), 0xffffffff}, specCount
                );
                m_selSlot->specRemoved += std::min(specModifyCount(), specCount);
            }
            break;
        }
        default: break;
        }
    }
}

void ItemUser::render(glm::vec2 relCursorPosPx, re::GeometryBatch& gb) {
    if (selItem().id != ItemID::Empty) {
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

int ItemUser::specModifyCount() const {
    switch (m_shape) {
    case World::ModificationShape::Square:
        return static_cast<int>(glm::pi<float>() * m_radiusTi * m_radiusTi);
    case World::ModificationShape::Disk:
        return static_cast<int>((m_radiusTi * 2.0f) * (m_radiusTi * 2.0f));
    case World::ModificationShape::Fill:
        return static_cast<int>(k_modifyTilesGroupSize * k_modifyTilesGroupSize);
    default: return 0;
    }
}

} // namespace rw
