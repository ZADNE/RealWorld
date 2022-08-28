/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>

#include <RealEngine/rendering/batches/GeometryBatch.hpp>

#include <RealWorld/world/World.hpp>
#include <RealWorld/items/Inventory.hpp>
#include <RealWorld/physics/Hitbox.hpp>

/**
 * @brief Uses items from an inventory.
 *
 * Can add or remove tile to the world.
*/
class ItemUser {
public:

    const static int PRIMARY_USE = 0;
    const static int SECONDARY_USE = 1;

    ItemUser(World& world, Inventory& inventory, Hitbox& operatorsHitbox);

    void switchShape();
    void resizeShape(float change);

    //Does not check if the slot is inside the inventory!
    void selectSlot(int slot);

    void step(bool usePrimary, bool useSecondary, const glm::ivec2& relCursorPosPx, RE::GeometryBatch& geometryBatch);

private:

    World& m_world;
    Inventory& m_inv;
    Hitbox& m_operatorsHitbox;

    int m_chosenSlot = 0;

    MODIFY_SHAPE m_shape = MODIFY_SHAPE::DISC;
    float m_diameter = 1.5f;

    // <  -1: steps not using
    // == -1: just stopped using
    // == +0: invalid state
    // >  +1: steps using
    // == +1: just started using
    int m_using[2] = {-1, -1};
    Item* m_item = nullptr;
};
