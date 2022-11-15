/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/drawing/WorldDrawer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/rendering/vertices/Vertex.hpp>

#include <RealWorld/constants/tile.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>


template<RE::Renderer R>
WorldDrawer<R>::WorldDrawer(const glm::uvec2& viewSizePx) :
    m_viewSizePx(viewSizePx),
    m_viewSizeTi(viewSizeTi(viewSizePx)),
    m_tileDrawer(m_viewSizeTi),
    m_shadowDrawer(m_viewSizeTi),
    m_minimapDrawer() {
}

template<RE::Renderer R>
void WorldDrawer<R>::setTarget(const glm::ivec2& worldTexSize) {
    m_worldTexSize = worldTexSize;
    m_minimapDrawer.setTarget(worldTexSize, m_viewSizePx);
    updateViewSizeDependentUniforms();
}

template<RE::Renderer R>
void WorldDrawer<R>::resizeView(const glm::uvec2& viewSizePx) {
    m_viewSizePx = viewSizePx;
    m_viewSizeTi = viewSizeTi(viewSizePx);
    updateViewSizeDependentUniforms();
    m_shadowDrawer.resizeView(m_viewSizeTi);
    m_minimapDrawer.resizeView(m_worldTexSize, m_viewSizePx);
}

template<RE::Renderer R>
WorldDrawer<R>::ViewEnvelope WorldDrawer<R>::setPosition(const glm::vec2& botLeftPx) {
    m_botLeftPx = botLeftPx;
    m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TILEPx));
    return ViewEnvelope{.botLeftTi = m_botLeftTi - glm::ivec2(LIGHT_MAX_RANGETi), .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) + glm::ivec2(LIGHT_MAX_RANGETi)};
}

template<RE::Renderer R>
void WorldDrawer<R>::beginStep() {
    m_shadowDrawer.analyze(m_uniformBuf, m_botLeftTi);
}

template<RE::Renderer R>
void WorldDrawer<R>::addExternalLight(const glm::ivec2& posPx, RE::Color col) {
    m_shadowDrawer.addExternalLight(posPx, col);
}

template<RE::Renderer R>
void WorldDrawer<R>::endStep() {
    m_shadowDrawer.calculate(m_uniformBuf, m_botLeftPx);
}

template<RE::Renderer R>
void WorldDrawer<R>::drawTiles() {
    m_tileDrawer.draw(m_uniformBuf, m_vao, m_botLeftPx, m_viewSizeTi);
}

template<RE::Renderer R>
void WorldDrawer<R>::drawShadows() {
    if (m_drawShadows) {
        m_shadowDrawer.draw(m_uniformBuf, m_vao, m_botLeftPx, m_viewSizeTi);
    }
}

template<RE::Renderer R>
void WorldDrawer<R>::drawMinimap() {
    if (m_drawMinimap) {
        m_minimapDrawer.draw();
    }
}

template<RE::Renderer R>
void WorldDrawer<R>::updateViewSizeDependentUniforms() {
    WorldDrawerUniforms wdu{
        .viewMat = glm::ortho(0.0f, m_viewSizePx.x, 0.0f, m_viewSizePx.y),
        .worldTexMask = m_worldTexSize - 1,
        .viewWidthTi = static_cast<int>(m_viewSizeTi.x)
    };
    m_uniformBuf.overwrite(0u, wdu);
}

template<RE::Renderer R>
glm::uvec2 WorldDrawer<R>::viewSizeTi(const glm::vec2& viewSizePx) const {
    return glm::uvec2(glm::ceil(viewSizePx / TILEPx)) + 1u;
}

template class WorldDrawer<RE::RendererVK13>;
template class WorldDrawer<RE::RendererGL46>;
