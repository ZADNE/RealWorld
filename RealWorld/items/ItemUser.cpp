/*! 
 *  @author    Dubsky Tomas
 */
#include <RealWorld/items/ItemUser.hpp>


template<RE::Renderer R>
ItemUser<R>::ItemUser(World<R>& world, Inventory<R>& inventory, Hitbox& operatorsHitbox) :
    m_world(world),
    m_inv(inventory),
    m_operatorsHitbox(operatorsHitbox) {

    m_item = &m_inv[m_chosenSlot][0];
}

template<RE::Renderer R>
void ItemUser<R>::switchShape() {
    m_shape = m_shape == MODIFY_SHAPE::DISC ? MODIFY_SHAPE::SQUARE : MODIFY_SHAPE::DISC;
}

template<RE::Renderer R>
void ItemUser<R>::resizeShape(float change) {
    m_diameter = glm::clamp(m_diameter + change, 0.5f, 7.5f);
}

template<RE::Renderer R>
void ItemUser<R>::selectSlot(int slot) {
    m_chosenSlot = slot;
    m_item = &m_inv[m_chosenSlot][0];
}

template<RE::Renderer R>
void ItemUser<R>::step(bool usePrimary, bool useSecondary, const glm::ivec2& relCursorPosPx, RE::GeometryBatch<R>& gb) {
    bool use[2] = {usePrimary, useSecondary};

    //Update usage
    for (int i = 0; i < 2; i++) {
        if (use[i] != (m_using[i] > 0)) {
            m_using[i] = use[i] ? +1 : -1;
        } else {
            m_using[i] += glm::sign(m_using[i]);
        }
    }

    const ItemMetadata& md = ItemDatabase::md(m_item->ID);

    //MAIN
    if (m_using[PRIMARY_USE] > 0) {
        switch (md.type) {
        case ITEM_TYPE::EMPTY:
            break;
        case ITEM_TYPE::PICKAXE:
            m_world.modify(LAYER::BLOCK, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(BLOCK::AIR, 0));
            break;
        case ITEM_TYPE::HAMMER:
            m_world.modify(LAYER::WALL, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(WALL::AIR, 0));
            break;
        }
    }
    //ALTERNATIVE
    if (m_using[SECONDARY_USE] > 0) {
        switch (md.type) {
        case ITEM_TYPE::EMPTY:
            break;
        case ITEM_TYPE::BLOCK:
            m_world.modify(LAYER::BLOCK, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(md.typeIndex, 256));
            break;
        case ITEM_TYPE::WALL:
            m_world.modify(LAYER::WALL, m_shape, m_diameter, pxToTi(relCursorPosPx), glm::uvec2(md.typeIndex, 256));
            break;
        }
    }

    
    if (md.type != ITEM_TYPE::EMPTY) {//Draw
        RE::Color col{255, 255, 255, 255};
        glm::vec2 c = tiToPx(pxToTi(relCursorPosPx)) + TILEPx * 0.5f;
        float dia = m_diameter * TILEPx.x;
        if (m_shape == MODIFY_SHAPE::DISC) {
            RE::CirclePOCO circ{RE::CirclePO{c, dia, false}, col};
            gb.addCircles(0u, 1u, &circ);
        } else {
            RE::VertexPOCO square[4] = {
                {c + glm::vec2{-dia, -dia}, col},
                {c + glm::vec2{+dia, -dia}, col},
                {c + glm::vec2{+dia, +dia}, col},
                {c + glm::vec2{-dia, +dia}, col}
            };
            gb.addPrimitives(RE::PRIM::LINE_LOOP, 0, 4, square);
        }
    }
}

template ItemUser<RE::RendererGL46>;
