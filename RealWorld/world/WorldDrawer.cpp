/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/WorldDrawer.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealWorld/chunk/ChunkManager.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/shaders/common.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>


WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx) :
	m_viewSizePx(viewSizePx),
	m_viewSizeTi(viewSizeTi(viewSizePx)),
	m_tileDrawer(m_viewSizeTi, m_uniformBuf),
	m_shadowDrawer(m_viewSizeTi, m_uniformBuf),
	m_minimapDrawer() {

	updateUniformsAfterViewResize();
}

WorldDrawer::~WorldDrawer() {

}

void WorldDrawer::setTarget(const glm::ivec2& worldDimTi) {
	m_minimapDrawer.setTarget(worldDimTi, m_viewSizePx);
}

void WorldDrawer::resizeView(const glm::uvec2& viewSizePx) {
	m_viewSizePx = viewSizePx;
	m_viewSizeTi = viewSizeTi(viewSizePx);
	updateUniformsAfterViewResize();
	m_shadowDrawer.resizeView(m_viewSizeTi);
	m_minimapDrawer.resizeView(m_viewSizePx);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(const glm::vec2& botLeftPx) {
	m_botLeftPx = botLeftPx;
	m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TILEPx)) - glm::ivec2(LIGHT_MAX_RANGETi);
	return ViewEnvelope{.botLeftTi = m_botLeftTi, .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) + glm::ivec2(LIGHT_MAX_RANGETi) * 2};
}

void WorldDrawer::beginStep() {
	m_shadowDrawer.analyze(m_botLeftTi);
}

void WorldDrawer::endStep() {
	m_shadowDrawer.calculate();
}

void WorldDrawer::drawTiles() {
	m_tileDrawer.draw(m_vao, m_botLeftPx, m_viewSizeTi);
}

void WorldDrawer::drawShadows() {
	if (m_drawShadows) {
		m_shadowDrawer.draw(m_vao, m_botLeftPx, m_viewSizeTi);
	}
}

void WorldDrawer::drawMinimap() {
	if (m_drawMinimap) {
		m_minimapDrawer.draw();
	}
}

void WorldDrawer::updateUniformsAfterViewResize() {
	WorldDrawerUniforms wdu{
		.viewsizePxMat = glm::ortho(0.0f, m_viewSizePx.x, 0.0f, m_viewSizePx.y),
		.viewWidthTi = static_cast<int>(m_viewSizeTi.x)
	};
	m_uniformBuf.overwrite(0u, wdu);
}

glm::uvec2 WorldDrawer::viewSizeTi(const glm::vec2& viewSizePx) const {
	return glm::uvec2(glm::ceil(viewSizePx / TILEPx)) + 1u;
}
