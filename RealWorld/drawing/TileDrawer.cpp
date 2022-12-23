/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/TileDrawer.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/buffers.hpp>

TileDrawer::TileDrawer(WorldDrawerPushConstants& pushConstants, const glm::uvec2& viewSizeTi) :
    m_pushConstants(pushConstants) {
}

void TileDrawer::draw(const vk::CommandBuffer& commandBuffer, const glm::vec2& botLeftPx, const glm::uvec2& viewSizeTi) {
    m_pushConstants.botLeftPxModTilePx = glm::mod(botLeftPx, TILEPx);
    m_pushConstants.botLeftTi = glm::ivec2(pxToTi(botLeftPx));
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_drawTilesPl);
    commandBuffer.pushConstants<WorldDrawerPushConstants>(m_drawTilesPl.pipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0u, m_pushConstants);
    commandBuffer.draw(4u, viewSizeTi.x * viewSizeTi.y, 0u, 0u);
}