/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/MinimapDrawer.hpp>

#include <RealWorld/drawing/shaders/AllShaders.hpp>

using enum vk::ShaderStageFlagBits;

MinimapDrawer::MinimapDrawer(const RE::PipelineLayout& pipelineLayout):
    m_drawMinimapPl(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *pipelineLayout,
            .topology = vk::PrimitiveTopology::eTriangleStrip
        }, RE::PipelineGraphicsSources{
            .vert = drawMinimap_vert,
            .frag = drawMinimap_frag
        }
    ) {
}

void MinimapDrawer::setTarget(const glm::ivec2& worldTexSize, const glm::vec2& viewSizePx) {
    //updateArrayBuffers(worldTexSize, viewSizePx);
}

void MinimapDrawer::resizeView(const glm::ivec2& worldTexSize, const glm::uvec2& viewSizePx) {
    //updateArrayBuffers(worldTexSize, viewSizePx);
}

void MinimapDrawer::draw(
    WorldDrawerPushConstants& pushConstants,
    const RE::PipelineLayout& pipelineLayout,
    const vk::CommandBuffer& commandBuffer
) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawMinimapPl);
    commandBuffer.pushConstants<WorldDrawerPushConstants>(*pipelineLayout, eVertex | eFragment, 0u, pushConstants);
    commandBuffer.draw(4u, 1u, 0u, 0u);
}
