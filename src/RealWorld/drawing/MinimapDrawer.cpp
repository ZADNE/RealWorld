/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/commands/CommandBuffer.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/drawing/MinimapDrawer.hpp>

namespace rw {

namespace {
constexpr re::Color k_red{255, 0, 0, 255};
constexpr re::Color k_green{0, 255, 0, 255};
} // namespace

MinimapDrawer::MinimapDrawer(
    re::RenderPassSubpass renderPassSubpass, glm::vec2 viewSizePx, glm::vec2 viewSizeTi
)
    : m_geometryBatch{re::GeometryBatchCreateInfo{
          .topology          = vk::PrimitiveTopology::eLineList,
          .renderPassSubpass = renderPassSubpass,
          .maxVertices       = 512
      }} {
    resizeView(viewSizePx, viewSizeTi);
}

void MinimapDrawer::setTarget(glm::vec2 worldTexSize) {
    m_worldTexCh = tiToCh(worldTexSize);

    resizeView(m_viewSizePx, m_viewSizeTi);
}

void MinimapDrawer::resizeView(glm::vec2 viewSizePx, glm::vec2 viewSizeTi) {
    m_viewMat    = glm::ortho(0.0f, viewSizePx.x, 0.0f, viewSizePx.y);
    m_viewSizePx = viewSizePx;
    m_viewSizeTi = viewSizeTi;

    m_layout = minimapLayout(chToTi(m_worldTexCh), m_viewSizePx);
}

void MinimapDrawer::drawMinimapLines(const re::CommandBuffer& cb, glm::vec2 botLeftPx) {
    m_geometryBatch.begin();
    std::array<re::VertexPoCo, 2> v = std::to_array(
        {re::VertexPoCo{glm::vec2{}, k_red}, re::VertexPoCo{glm::vec2{}, k_red}}
    );

    // Horizontal lines
    v[0].position.x = m_layout.offsetPx.x;
    v[1].position.x = m_layout.offsetPx.x + m_layout.sizePx.x;
    for (int y = 0; y <= m_worldTexCh.y; ++y) {
        float fy = m_layout.offsetPx.y +
                   (static_cast<float>(y) / m_worldTexCh.y) * m_layout.sizePx.y;
        v[0].position.y = fy;
        v[1].position.y = fy;
        m_geometryBatch.addVertices(v);
    }

    // Vertical lines
    v[0].position.y = m_layout.offsetPx.y;
    v[1].position.y = m_layout.offsetPx.y + m_layout.sizePx.y;
    for (int x = 0; x <= m_worldTexCh.x; ++x) {
        float fx = m_layout.offsetPx.x +
                   (static_cast<float>(x) / m_worldTexCh.x) * m_layout.sizePx.x;
        v[0].position.x = fx;
        v[1].position.x = fx;
        m_geometryBatch.addVertices(v);
    }

    { // View
        v[0].color        = k_green;
        v[1].color        = k_green;
        glm::vec2  scale  = m_layout.sizePx / chToPx(m_worldTexCh);
        glm::ivec2 mask   = chToPx(m_worldTexCh) - 1.0f;
        auto       toView = [&](glm::vec2 worldPx) -> glm::vec2 {
            return worldPx * scale + m_layout.offsetPx;
        };
        auto addRectangle = [&](glm::vec2 bl, glm::vec2 size) {
            for (float x = 0.0; x <= m_layout.sizePx.x; x += m_layout.sizePx.x) {
                for (float y = 0.0; y <= m_layout.sizePx.y;
                     y += m_layout.sizePx.y) {
                    v[0].position = toView(bl + glm::vec2{0.0, 0.0}) -
                                    glm::vec2{x, y};
                    v[1].position = toView(bl + glm::vec2{size.x, 0.0f}) -
                                    glm::vec2{x, y};
                    m_geometryBatch.addVertices(v);
                    v[0].position = v[1].position;
                    v[1].position = toView(bl + size) - glm::vec2{x, y};
                    m_geometryBatch.addVertices(v);
                    v[0].position = v[1].position;
                    v[1].position = toView(bl + glm::vec2{0.0, size.y}) -
                                    glm::vec2{x, y};
                    m_geometryBatch.addVertices(v);
                    v[0].position = v[1].position;
                    v[1].position = toView(bl + glm::vec2{0.0, 0.0}) -
                                    glm::vec2{x, y};
                    m_geometryBatch.addVertices(v);
                }
            }
        };

        // Normal view
        addRectangle(static_cast<glm::ivec2>(botLeftPx) & mask, m_viewSizePx);

        // View extended by light range
        addRectangle(
            static_cast<glm::ivec2>(
                botLeftPx - tiToPx(glm::vec2{k_lightMaxRangeTi + iChunkTi})
            ) & mask,
            m_viewSizePx + tiToPx(glm::vec2{k_lightMaxRangeTi + iChunkTi}) * 2.0f
        );
    }

    m_geometryBatch.end();
    cb->setScissor(
        0,
        vk::Rect2D{
            vk::Offset2D{
                static_cast<int32_t>(m_layout.offsetPx.x - 1),
                static_cast<int32_t>(m_layout.offsetPx.y - 1)
            },
            vk::Extent2D{
                static_cast<uint32_t>(m_layout.sizePx.x + 1),
                static_cast<uint32_t>(m_layout.sizePx.y + 1)
            }
        }
    );
    m_geometryBatch.draw(cb, m_viewMat);
    cb->setScissor(
        0,
        vk::Rect2D{
            vk::Offset2D{},
            vk::Extent2D{
                static_cast<uint32_t>(m_viewSizePx.x),
                static_cast<uint32_t>(m_viewSizePx.y)
            }
        }
    );
}

} // namespace rw
