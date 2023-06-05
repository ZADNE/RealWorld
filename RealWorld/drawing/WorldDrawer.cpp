/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/graphics/pipelines/Vertex.hpp>

#include <RealWorld/constants/light.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/WorldDrawer.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;

namespace rw {

WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx, glm::uint maxNumberOfExternalLights)
    : m_viewSizeTi(viewSizeTi(viewSizePx))
    , m_tileDrawer(viewSizePx, m_viewSizeTi)
    , m_shadowDrawer(viewSizePx, m_viewSizeTi, maxNumberOfExternalLights) {
}

void WorldDrawer::setTarget(
    const re::Texture& worldTexture, const glm::ivec2& worldTexSize
) {
    m_tileDrawer.setTarget(worldTexture, worldTexSize);
    m_shadowDrawer.setTarget(worldTexture, worldTexSize);
}

void WorldDrawer::resizeView(const glm::uvec2& viewSizePx) {
    m_viewSizeTi = viewSizeTi(viewSizePx);
    m_tileDrawer.resizeView(viewSizePx, m_viewSizeTi);
    m_shadowDrawer.resizeView(viewSizePx, m_viewSizeTi);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(const glm::vec2& botLeftPx) {
    m_botLeftPx = botLeftPx;
    m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TilePx));
    return ViewEnvelope{
        .botLeftTi  = m_botLeftTi - glm::ivec2(k_lightMaxRangeTi),
        .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) +
                      glm::ivec2(k_lightMaxRangeTi)};
}

void WorldDrawer::beginStep(const vk::CommandBuffer& commandBuffer) {
    m_shadowDrawer.analyze(commandBuffer, m_botLeftTi);
}

void WorldDrawer::addExternalLight(const glm::ivec2& posPx, re::Color col) {
    m_shadowDrawer.addExternalLight(posPx, col);
}

void WorldDrawer::endStep(const vk::CommandBuffer& commandBuffer) {
    m_shadowDrawer.calculate(commandBuffer, m_botLeftPx);
}

void WorldDrawer::drawTiles(const vk::CommandBuffer& commandBuffer) {
    m_tileDrawer.drawTiles(commandBuffer, m_botLeftPx);
}

void WorldDrawer::drawShadows(const vk::CommandBuffer& commandBuffer) {
    m_shadowDrawer.draw(commandBuffer, m_botLeftPx, m_viewSizeTi);
}

void WorldDrawer::drawMinimap(const vk::CommandBuffer& commandBuffer) {
    m_tileDrawer.drawMinimap(commandBuffer);
}

glm::uvec2 WorldDrawer::viewSizeTi(const glm::vec2& viewSizePx) const {
    return glm::uvec2(glm::ceil(viewSizePx / TilePx)) + 1u;
}

} // namespace rw
