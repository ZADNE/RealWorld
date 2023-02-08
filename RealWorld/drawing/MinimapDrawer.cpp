/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/MinimapDrawer.hpp>

#include <RealWorld/drawing/shaders/AllShaders.hpp>


MinimapDrawer::MinimapDrawer(const RE::PipelineLayout& pipelineLayout):
    m_drawMinimapPl(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *pipelineLayout,
            .topology = vk::PrimitiveTopology::eTriangleStrip
        }, RE::PipelineGraphicsSources{
            .vert = drawTiles_vert,
            .frag = drawColor_frag
        }
    ) {
}

void MinimapDrawer::setTarget(const glm::ivec2& worldTexSize, const glm::vec2& viewSizePx) {
    //updateArrayBuffers(worldTexSize, viewSizePx);
}

void MinimapDrawer::resizeView(const glm::ivec2& worldTexSize, const glm::uvec2& viewSizePx) {
    //updateArrayBuffers(worldTexSize, viewSizePx);
}

void MinimapDrawer::draw(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.draw(4u, 1u, 0u, 0u);
}
