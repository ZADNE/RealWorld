/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/graphics/batches/GeometryBatch.hpp>

#include <RealWorld/items/Inventory.hpp>
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

    // Does not check if the slot is inside the inventory!
    void selectSlot(int slot);

    void step(
        const vk::CommandBuffer& commandBuffer,
        bool                     usePrimary,
        bool                     useSecondary,
        const glm::ivec2&        relCursorPosPx
    );

    void render(const glm::vec2& relCursorPosPx, re::GeometryBatch& gb);

private:
    World&     m_world;
    Inventory& m_inv;

    int m_chosenSlot = 0;

    World::ModificationShape m_shape    = World::ModificationShape::Disk;
    float                    m_radiusTi = 1.5f;

    // <  -1: steps not using
    // == -1: just stopped using
    // == +0: invalid state
    // >  +1: steps using
    // == +1: just started using
    int   m_using[2] = {-1, -1};
    Item* m_item     = nullptr;
};

} // namespace rw
