/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/WorldDrawer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/rendering/pipelines/Vertex.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>


WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx):
    m_viewSizePx(viewSizePx),
    m_viewSizeTi(viewSizeTi(viewSizePx)),
    m_tileDrawer(m_pipelineLayout, m_descriptorSet)/*,
    m_shadowDrawer(m_viewSizeTi),
    m_minimapDrawer()*/ {
}

void WorldDrawer::setTarget(const RE::Texture& worldTexture, const glm::ivec2& worldTexSize) {
    m_worldTexSize = worldTexSize;
    m_descriptorSet.write(vk::DescriptorType::eCombinedImageSampler, 0u, 0u, worldTexture);
    //m_minimapDrawer.setTarget(worldTexSize, m_viewSizePx);
    updateViewSizeDependentUniforms();
}

void WorldDrawer::resizeView(const glm::uvec2& viewSizePx) {
    m_viewSizePx = viewSizePx;
    m_viewSizeTi = viewSizeTi(viewSizePx);
    updateViewSizeDependentUniforms();
    //m_shadowDrawer.resizeView(m_viewSizeTi);
    //m_minimapDrawer.resizeView(m_worldTexSize, m_viewSizePx);
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
    //m_shadowDrawer.analyze(m_uniformBuf, m_botLeftTi);
}

void WorldDrawer::addExternalLight(const glm::ivec2& posPx, RE::Color col) {
    //m_shadowDrawer.addExternalLight(posPx, col);
}

void WorldDrawer::endStep() {
    //m_shadowDrawer.calculate(m_uniformBuf, m_botLeftPx);
}

void WorldDrawer::beginDrawing(const vk::CommandBuffer& commandBuffer) {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {});
}

void WorldDrawer::drawTiles(const vk::CommandBuffer& commandBuffer) {
    m_tileDrawer.draw(m_pushConstants, m_pipelineLayout, commandBuffer, m_botLeftPx, m_viewSizeTi);
}

void WorldDrawer::drawShadows(const vk::CommandBuffer& commandBuffer) {
    //m_shadowDrawer.draw(commandBuffer, m_uniformBuf, m_botLeftPx, m_viewSizeTi);
}

void WorldDrawer::drawMinimap(const vk::CommandBuffer& commandBuffer) {
    //m_minimapDrawer.draw(commandBuffer);
}

void WorldDrawer::updateViewSizeDependentUniforms() {
    m_pushConstants.viewMat = glm::ortho(0.0f, m_viewSizePx.x, 0.0f, m_viewSizePx.y);
    m_pushConstants.worldTexMask = m_worldTexSize - 1;
    m_pushConstants.viewWidthTi = static_cast<int>(m_viewSizeTi.x);
}

glm::uvec2 WorldDrawer::viewSizeTi(const glm::vec2& viewSizePx) const {
    return glm::uvec2(glm::ceil(viewSizePx / TILEPx)) + 1u;
}
