/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>
#include <cstdint>
#include <string>

#include <RealEngine/resources/ResourceID.hpp>

#include <RealWorld/constants/item.hpp>

namespace rw {

ItemIDSection section(ItemID id);

uint16_t offsetInSection(ItemID id);

/**
 * @brief Returns the maximum number of items of this id that can be stacked in a slot
 */
int maxStack(ItemID id);

/**
 * @brief Returns resource ID of the texture that should be used to render the item.
 */
re::ResourceID textureID(ItemID id);

/**
 * @brief Returns sprite index in the texture atlas
 */
float spriteIndex(ItemID id);

/**
 * @brief Returns subimage index of the sprite
 */
float subimageIndex(ItemID id);

/**
 * @brief Returns draw scale
 */
float drawScale(ItemID id);

} // namespace rw
