/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>

namespace rw {

constexpr int k_maxDroppedTilesCount = 2047;

struct alignas(sizeof(glm::vec2)) DroppedTile {
    glm::vec2 botLeftPx;
    glm::vec2 velPx;
    glm::uint lifetimeSteps;
    glm::uint layerTypeVar;
};

struct DroppedTilesSBHeader {
    vk::DrawIndirectCommand drawCommand{0, 1, 0, 0};
    vk::DispatchIndirectCommand dispatchCommand{0, 1, 1};
    glm::uint padding;
};

struct DroppedTilesSB {
    DroppedTilesSBHeader header;
    DroppedTile tiles[k_maxDroppedTilesCount];
};

} // namespace rw
