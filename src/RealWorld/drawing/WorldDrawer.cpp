/**
 *  @author    Dubsky Tomas
 */
#include <glm/gtc/constants.hpp>
#include <glm/packing.hpp>
#include <glm/trigonometric.hpp>

#include <RealEngine/graphics/pipelines/Vertex.hpp>

#include <RealWorld/constants/Chunk.hpp>
#include <RealWorld/constants/Tile.hpp>
#include <RealWorld/drawing/WorldDrawer.hpp>

using enum vk::DescriptorType;
using enum vk::ShaderStageFlagBits;

namespace rw {

constexpr float k_maxDaylightPower = 0.14f;

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

void WorldDrawer::setTarget(
    const re::Texture& worldTex, glm::ivec2 worldTexSizeTi, float seed
) {
    m_tileDrawer.setTarget(worldTex, worldTexSizeTi, seed);
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
    m_botLeftTi = pxToTi(glm::ivec2{botLeftPx});
    return ViewEnvelope{
        .botLeftTi = m_botLeftTi - glm::ivec2(glsl::k_lightMaxRangeTi) - iChunkTi,
        .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) +
                      glm::ivec2(glsl::k_lightMaxRangeTi) + iChunkTi
    };
}

void WorldDrawer::beginStep(const re::CommandBuffer& cb, float timeD) {
    m_timeD = timeD;
    m_shadowDrawer.analyze(cb, m_botLeftTi, timeD);
}

void WorldDrawer::addExternalLight(glm::ivec2 posPx, glm::vec3 light) {
    m_shadowDrawer.addExternalLight(posPx, light);
}

void WorldDrawer::endStep(const re::CommandBuffer& cb) {
    m_shadowDrawer.calculate(cb, m_botLeftPx);
}

void WorldDrawer::drawTiles(const re::CommandBuffer& cb) {
    m_tileDrawer.drawTiles(cb, m_botLeftPx, m_timeD);
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

} // namespace rw
