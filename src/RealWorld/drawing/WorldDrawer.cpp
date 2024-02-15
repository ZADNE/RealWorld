﻿/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/graphics/pipelines/Vertex.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/WorldDrawer.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;

namespace rw {

WorldDrawer::WorldDrawer(glm::uvec2 viewSizePx, glm::uint maxNumberOfExternalLights)
    : m_viewSizeTi(viewSizeTi(viewSizePx))
    , m_tileDrawer(viewSizePx, m_viewSizeTi)
    , m_shadowDrawer(viewSizePx, m_viewSizeTi, maxNumberOfExternalLights)
    , m_minimapDawer(viewSizePx, m_viewSizeTi) {
}

void WorldDrawer::setTarget(const re::Texture& worldTex, glm::ivec2 worldTexSizeTi) {
    m_tileDrawer.setTarget(worldTex, worldTexSizeTi);
    m_shadowDrawer.setTarget(worldTex, worldTexSizeTi);
    m_minimapDawer.setTarget(worldTexSizeTi);
}

void WorldDrawer::resizeView(glm::uvec2 viewSizePx) {
    m_viewSizeTi = viewSizeTi(viewSizePx);
    m_tileDrawer.resizeView(viewSizePx, m_viewSizeTi);
    m_shadowDrawer.resizeView(viewSizePx, m_viewSizeTi);
    m_minimapDawer.resizeView(viewSizePx, m_viewSizeTi);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(glm::vec2 botLeftPx) {
    m_botLeftPx = botLeftPx;
    m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TilePx));
    return ViewEnvelope{
        .botLeftTi  = m_botLeftTi - glm::ivec2(k_lightMaxRangeTi) - iChunkTi,
        .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) +
                      glm::ivec2(k_lightMaxRangeTi) + iChunkTi};
}

void WorldDrawer::beginStep(const re::CommandBuffer& cmdBuf) {
    m_shadowDrawer.analyze(cmdBuf, m_botLeftTi);
}

void WorldDrawer::addExternalLight(glm::ivec2 posPx, re::Color col) {
    m_shadowDrawer.addExternalLight(posPx, col);
}

void WorldDrawer::endStep(const re::CommandBuffer& cmdBuf) {
    m_shadowDrawer.calculate(cmdBuf, m_botLeftPx);
}

void WorldDrawer::drawTiles(const re::CommandBuffer& cmdBuf) {
    m_tileDrawer.drawTiles(cmdBuf, m_botLeftPx);
}

void WorldDrawer::drawShadows(const re::CommandBuffer& cmdBuf) {
    m_shadowDrawer.draw(cmdBuf, m_botLeftPx, m_viewSizeTi);
}

void WorldDrawer::drawMinimap(const re::CommandBuffer& cmdBuf) {
    m_tileDrawer.drawMinimap(cmdBuf);
    m_minimapDawer.drawMinimapLines(cmdBuf, m_botLeftPx);
}

glm::uvec2 WorldDrawer::viewSizeTi(glm::vec2 viewSizePx) const {
    return glm::uvec2(glm::ceil(viewSizePx / TilePx)) + 1u;
}

} // namespace rw
