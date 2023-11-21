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

WorldDrawer::WorldDrawer(glm::uvec2 viewSizePx, glm::uint maxNumberOfExternalLights)
    : m_viewSizeTi(viewSizeTi(viewSizePx))
    , m_tileDrawer(viewSizePx, m_viewSizeTi)
    , m_shadowDrawer(viewSizePx, m_viewSizeTi, maxNumberOfExternalLights) {
}

void WorldDrawer::setTarget(const re::Texture& worldTex, glm::ivec2 worldTexSizeTi) {
    m_tileDrawer.setTarget(worldTex, worldTexSizeTi);
    m_shadowDrawer.setTarget(worldTex, worldTexSizeTi);
}

void WorldDrawer::resizeView(glm::uvec2 viewSizePx) {
    m_viewSizeTi = viewSizeTi(viewSizePx);
    m_tileDrawer.resizeView(viewSizePx, m_viewSizeTi);
    m_shadowDrawer.resizeView(viewSizePx, m_viewSizeTi);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(glm::vec2 botLeftPx) {
    m_botLeftPx = botLeftPx;
    m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TilePx));
    return ViewEnvelope{
        .botLeftTi  = m_botLeftTi - glm::ivec2(k_lightMaxRangeTi),
        .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) +
                      glm::ivec2(k_lightMaxRangeTi)};
}

void WorldDrawer::beginStep(const vk::CommandBuffer& cmdBuf) {
    m_shadowDrawer.analyze(cmdBuf, m_botLeftTi);
}

void WorldDrawer::addExternalLight(glm::ivec2 posPx, re::Color col) {
    m_shadowDrawer.addExternalLight(posPx, col);
}

void WorldDrawer::endStep(const vk::CommandBuffer& cmdBuf) {
    m_shadowDrawer.calculate(cmdBuf, m_botLeftPx);
}

void WorldDrawer::drawTiles(const vk::CommandBuffer& cmdBuf) {
    m_tileDrawer.drawTiles(cmdBuf, m_botLeftPx);
}

void WorldDrawer::drawShadows(const vk::CommandBuffer& cmdBuf) {
    m_shadowDrawer.draw(cmdBuf, m_botLeftPx, m_viewSizeTi);
}

void WorldDrawer::drawMinimap(const vk::CommandBuffer& cmdBuf) {
    m_tileDrawer.drawMinimap(cmdBuf);
}

glm::uvec2 WorldDrawer::viewSizeTi(glm::vec2 viewSizePx) const {
    return glm::uvec2(glm::ceil(viewSizePx / TilePx)) + 1u;
}

} // namespace rw
