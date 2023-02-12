/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/WorldDrawer.hpp>

#include <RealEngine/rendering/pipelines/Vertex.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;

WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx, glm::uint maxNumberOfExternalLights):
    m_viewSizeTi(viewSizeTi(viewSizePx)),
    m_tileDrawer(viewSizePx, m_viewSizeTi),
    m_shadowDrawer(m_viewSizeTi, maxNumberOfExternalLights) {
}

void WorldDrawer::setTarget(const RE::Texture& worldTexture, const glm::ivec2& worldTexSize) {
    m_tileDrawer.setTarget(worldTexture, worldTexSize);
}

void WorldDrawer::resizeView(const glm::uvec2& viewSizePx) {
    m_viewSizeTi = viewSizeTi(viewSizePx);
    m_tileDrawer.resizeView(viewSizePx, m_viewSizeTi);
    //m_shadowDrawer.resizeView(m_viewSizeTi);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(const glm::vec2& botLeftPx) {
    m_botLeftPx = botLeftPx;
    m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TILEPx));
    return ViewEnvelope{
        .botLeftTi = m_botLeftTi - glm::ivec2(LIGHT_MAX_RANGETi),
        .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) + glm::ivec2(LIGHT_MAX_RANGETi)
    };
}

void WorldDrawer::beginStep(const vk::CommandBuffer& commandBuffer) {
    m_shadowDrawer.analyze(commandBuffer, m_botLeftTi);
}

void WorldDrawer::addExternalLight(const glm::ivec2& posPx, RE::Color col) {
    m_shadowDrawer.addExternalLight(posPx, col);
}

void WorldDrawer::endStep(const vk::CommandBuffer& commandBuffer) {
    m_shadowDrawer.calculate(commandBuffer, m_botLeftTi);
}

void WorldDrawer::drawTiles(const vk::CommandBuffer& commandBuffer) {
    m_tileDrawer.drawTiles(commandBuffer, m_botLeftPx);
}

void WorldDrawer::drawShadows(const vk::CommandBuffer& commandBuffer) {
    //m_shadowDrawer.draw(m_botLeftPx, m_viewSizeTi);
}

void WorldDrawer::drawMinimap(const vk::CommandBuffer& commandBuffer) {
    m_tileDrawer.drawMinimap(commandBuffer);
}

glm::uvec2 WorldDrawer::viewSizeTi(const glm::vec2& viewSizePx) const {
    return glm::uvec2(glm::ceil(viewSizePx / TILEPx)) + 1u;
}
