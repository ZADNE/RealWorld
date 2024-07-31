/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/batches/GeometryBatch.hpp>

#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/main/ActionCmdBuf.hpp>
#include <RealWorld/world/World.hpp>

namespace rw {

/**
 * @brief Uses items from an inventory.
 * @details Adds or removes tile of the world.
 */
class ItemUser {
public:
    static constexpr int k_primaryUse   = 0;
    static constexpr int k_secondaryUse = 1;

    ItemUser(World& world, Inventory& inventory);

    void switchShape();
    void resizeShape(float change);

    /**
     * @brief Finishes speculative removal
     * @note  Must be called before step(...)
     */
    void finishSpecRemoval(int count);

    void step(
        const ActionCmdBuf& acb, int selSlot, bool usePrimary,
        bool useSecondary, glm::ivec2 relCursorPosPx
    );

    void render(glm::vec2 relCursorPosPx, re::GeometryBatch& gb);

private:
    Item& selItem() { return m_inv[m_selSlot->slotIndex][0]; }
    const Item& selItem() const { return m_inv[m_selSlot->slotIndex][0]; }
    int selItemSpecCount() const {
        int count = selItem().count;
        if (m_selSlot.write().slotIndex == m_selSlot.read().slotIndex) {
            count -= m_selSlot.read().specRemoved; // Speculatively removed last step
        }
        return count;
    }

    World& m_world;
    Inventory& m_inv;

    struct SelSlotState {
        int slotIndex;
        int specRemoved{};
    };
    re::StepDoubleBuffered<SelSlotState> m_selSlot{};

    World::ModificationShape m_shape = World::ModificationShape::Disk;
    float m_radiusTi                 = 1.5f;

    int specModifyCount() const;

    // <  -1: steps not using
    // == -1: just stopped using
    // == +0: invalid state
    // >  +1: steps using
    // == +1: just started using
    int m_using[2] = {-1, -1};
};

} // namespace rw
