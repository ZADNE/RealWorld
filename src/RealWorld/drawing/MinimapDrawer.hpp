/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <RealEngine/graphics/batches/GeometryBatch.hpp>

#include <RealWorld/drawing/MinimapLayout.hpp>

namespace rw {

/**
 * @brief Renders guiding lines on top of minimap
 */
class MinimapDrawer {
public:
    MinimapDrawer(
        re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx,
        glm::vec2 viewSizeTi
    );

    void setTarget(glm::vec2 worldTexSize);

    void resizeView(glm::vec2 viewSizePx, glm::vec2 viewSizeTi);

    void drawMinimapLines(const re::CommandBuffer& cb, glm::vec2 botLeftPx);

private:
    glm::mat4 m_viewMat{};
    glm::vec2 m_viewSizePx{};
    glm::vec2 m_viewSizeTi{};
    glm::vec2 m_worldTexCh{};
    re::GeometryBatch m_geometryBatch;
    MinimapLayout m_layout{};
};

} // namespace rw
