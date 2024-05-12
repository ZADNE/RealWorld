/*!
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/constants.hpp>
#include <glm/packing.hpp>
#include <glm/trigonometric.hpp>

#include <RealEngine/graphics/pipelines/Vertex.hpp>

#include <RealWorld/constants/chunk.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/constants/tile.hpp>
#include <RealWorld/drawing/WorldDrawer.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;

namespace rw {

WorldDrawer::WorldDrawer(
    re::RenderPassSubpass renderPassSubpass, glm::uvec2 viewSizePx,
    glm::uint maxNumberOfExternalLights
)
    : m_viewSizeTi(viewSizeTi(viewSizePx))
    , m_tileDrawer(renderPassSubpass, viewSizePx, m_pc)
    , m_shadowDrawer(
          renderPassSubpass, viewSizePx, m_viewSizeTi, maxNumberOfExternalLights, m_pc
      )
    , m_minimapDawer(renderPassSubpass, viewSizePx, m_viewSizeTi) {
}

void WorldDrawer::setTarget(const re::Texture& worldTex, glm::ivec2 worldTexSizeTi) {
    m_tileDrawer.setTarget(worldTex, worldTexSizeTi);
    m_shadowDrawer.setTarget(worldTex, worldTexSizeTi);
    m_minimapDawer.setTarget(worldTexSizeTi);
}

void WorldDrawer::resizeView(glm::uvec2 viewSizePx) {
    m_viewSizeTi = viewSizeTi(viewSizePx);
    m_tileDrawer.resizeView(viewSizePx);
    m_shadowDrawer.resizeView(viewSizePx, m_viewSizeTi);
    m_minimapDawer.resizeView(viewSizePx, m_viewSizeTi);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(glm::vec2 botLeftPx) {
    m_botLeftPx = botLeftPx;
    m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TilePx));
    return ViewEnvelope{
        .botLeftTi  = m_botLeftTi - glm::ivec2(k_lightMaxRangeTi) - iChunkTi,
        .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) +
                      glm::ivec2(k_lightMaxRangeTi) + iChunkTi
    };
}

void WorldDrawer::beginStep(const re::CommandBuffer& cb, float timeDay) {
    m_skyLightPower     = timeToSkyLightPower(timeDay);
    float skyLightPower = m_skyLightPower * m_skyLightPower * 0.14f;
    glm::vec4 skyLight  = glm::vec4{0.0f, 0.0f, 0.0f, skyLightPower};
    m_shadowDrawer.analyze(cb, m_botLeftTi, skyLight);
}

void WorldDrawer::addExternalLight(glm::ivec2 posPx, re::Color col) {
    m_shadowDrawer.addExternalLight(posPx, col);
}

void WorldDrawer::endStep(const re::CommandBuffer& cb) {
    m_shadowDrawer.calculate(cb, m_botLeftPx);
}

void WorldDrawer::drawTiles(const re::CommandBuffer& cb) {
    float skyLight = 0.0625f + glm::sqrt(m_skyLightPower) * 0.9375f;
    m_tileDrawer.drawTiles(cb, m_botLeftPx, skyLight);
}

void WorldDrawer::drawShadows(const re::CommandBuffer& cb) {
    m_shadowDrawer.draw(cb, m_botLeftPx);
}

void WorldDrawer::drawMinimap(const re::CommandBuffer& cb) {
    m_tileDrawer.drawMinimap(cb);
    m_minimapDawer.drawMinimapLines(cb, m_botLeftPx);
}

glm::uvec2 WorldDrawer::viewSizeTi(glm::vec2 viewSizePx) const {
    return glm::uvec2(glm::ceil(viewSizePx / TilePx)) + 1u;
}

float WorldDrawer::timeToSkyLightPower(float timeDay) const {
    const float s = glm::sin(timeDay * glm::pi<float>() * 2.0f);
    return glm::clamp(s * 0.75f + 0.5f, 0.0f, 1.0f);
}

} // namespace rw
