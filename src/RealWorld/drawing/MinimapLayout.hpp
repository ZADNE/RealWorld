/**
 *  @author    Dubsky Tomas
 */
#pragma once
#include <glm/common.hpp>
#include <glm/vec2.hpp>

namespace rw {

struct MinimapLayout {
    glm::vec2 offsetPx;
    glm::vec2 sizePx;
};

/**
 * @brief Calculates where minimap will be placed on the screen
 */
constexpr MinimapLayout minimapLayout(
    glm::vec2 worldTexSize, glm::vec2 viewSizePx, float padding = 100.0f
) {
    MinimapLayout layout{};
    float worldTexSizeRatio = worldTexSize.x / worldTexSize.y;
    if (worldTexSizeRatio > 1.0f) {
        float longerDim = viewSizePx.x - padding;
        layout.sizePx   = glm::vec2(longerDim, longerDim / worldTexSizeRatio);
    } else if (worldTexSizeRatio < 1.0f) {
        float longerDim = viewSizePx.y - padding;
        layout.sizePx   = glm::vec2(longerDim * worldTexSizeRatio, longerDim);
    } else {
        float longerDim = glm::min(viewSizePx.x - padding, viewSizePx.y - padding);
        layout.sizePx = glm::vec2(longerDim * worldTexSizeRatio, longerDim);
    }
    layout.offsetPx = viewSizePx * 0.5f - layout.sizePx * 0.5f;
    return layout;
}

} // namespace rw
