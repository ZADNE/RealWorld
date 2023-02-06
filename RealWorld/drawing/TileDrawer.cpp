/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/TileDrawer.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/buffers.hpp>


TileDrawer::TileDrawer(const RE::PipelineLayout& pipelineLayout)
    : m_drawTilesPl(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *pipelineLayout,
            .topology = vk::PrimitiveTopology::eTriangleStrip
        }, RE::PipelineGraphicsSources{
            .vert = drawTiles_vert, .frag = drawColor_frag
        }
    ) {

}

void TileDrawer::draw(
    WorldDrawerPushConstants& pushConstants,
    RE::PipelineLayout& pipelineLayout,
    const vk::CommandBuffer& commandBuffer,
    const glm::vec2& botLeftPx,
    const glm::uvec2& viewSizeTi
) {
    pushConstants.botLeftPxModTilePx = glm::mod(botLeftPx, TILEPx);
    pushConstants.botLeftTi = glm::ivec2(pxToTi(botLeftPx));
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    commandBuffer.pushConstants<WorldDrawerPushConstants>(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0u, pushConstants);
    commandBuffer.draw(4u, viewSizeTi.x * viewSizeTi.y, 0u, 0u);
}