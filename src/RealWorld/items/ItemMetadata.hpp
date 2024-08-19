/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>
#include <cstdint>
#include <string>

#include <RealWorld/constants/item.hpp>

namespace rw {

ItemIdSection section(ItemId id);

uint16_t offsetInSection(ItemId id);

/**
 * @brief Returns the maximum number of items of this id that can be stacked in a slot
 */
int maxStack(ItemId id);

/**
 * @brief Returns suffix of the texture atlas
 */
char textureAtlas(ItemId id);

/**
 * @brief Returns sprite index in the texture atlas
 */
float spriteIndex(ItemId id);

/**
 * @brief Returns subimage index of the sprite
 */
float subimageIndex(ItemId id);

/**
 * @brief Returns draw scale
 */
float drawScale(ItemId id);

} // namespace rw
