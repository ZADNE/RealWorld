/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/WorldDrawer.hpp>

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/Viewport.hpp>

#include <RealWorld/chunk/ChunkManager.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/shaders/common.hpp>
#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

static inline const RE::TextureFlags R8_NU_NEAR_LIN_EDGE{
	RE::TextureChannels::R, RE::TextureFormat::NORMALIZED_UNSIGNED, RE::TextureMinFilter::NEAREST_NO_MIPMAPS,
	RE::TextureMagFilter::LINEAR, RE::TextureWrapStyle::CLAMP_TO_EDGE, RE::TextureWrapStyle::CLAMP_TO_EDGE,
	RE::TextureBitdepthPerChannel::BITS_8
};

const GLint VERTICES_POUV_NORM_RECT = 0;
const GLint VERTICES_POUV_MINIMAP_RECT = 4;


WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx) :
	m_viewSizePx(viewSizePx),
	m_viewSizeTi(viewSizeTi(viewSizePx)),
	m_analysisGroupCount(analysisGroupCount(m_viewSizeTi)),
	m_analysisTex({glm::vec2(m_analysisGroupCount) * ANALYSIS_GROUP_SIZE * ANALYSIS_PER_THREAD_AREA}, {R8_NU_NEAR_LIN_EDGE}),
	m_calcShadowsGroupCount(calcShadowsGroupCount(m_viewSizeTi)),
	m_shadowsTex({glm::vec2(m_calcShadowsGroupCount) * CALC_GROUP_SIZE}, {RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE}) {

	//Bind textures to their reserved texture units
	m_blockAtlasTex->bind(TEX_UNIT_BLOCK_ATLAS);
	m_wallAtlasTex->bind(TEX_UNIT_WALL_ATLAS);
	m_blockLightAtlasTex->bind(TEX_UNIT_BLOCK_LIGHT_ATLAS);
	m_wallLightAtlasTex->bind(TEX_UNIT_WALL_LIGHT_ATLAS);

	m_analysisTex.bind(TEX_UNIT_TILE_TRANSLU);
	m_shadowsTex.bind(TEX_UNIT_SHADOWS);
	m_analysisTex.bindImage(IMG_UNIT_TILE_TRANSLU, 0, RE::ImageAccess::READ_WRITE);
	m_shadowsTex.bindImage(IMG_UNIT_SHADOWS, 0, RE::ImageAccess::READ_WRITE);

	initShaders();
	initVAOs();
	updateArrayBuffers();
}

WorldDrawer::~WorldDrawer() {

}

void WorldDrawer::setTarget(const glm::ivec2& worldDimTi) {
	m_worldDimTi = worldDimTi;
	updateArrayBuffers();
}

void WorldDrawer::resizeView(const glm::uvec2& newViewSizePx) {
	m_viewSizePx = newViewSizePx;
	m_viewSizeTi = viewSizeTi(newViewSizePx);
	m_analysisGroupCount = analysisGroupCount(m_viewSizeTi);
	m_analysisTex = RE::Texture({glm::vec2(m_analysisGroupCount) * ANALYSIS_GROUP_SIZE * ANALYSIS_PER_THREAD_AREA}, {R8_NU_NEAR_LIN_EDGE});
	m_shadowsTex = RE::Texture({glm::vec2(m_calcShadowsGroupCount) * CALC_GROUP_SIZE}, {RE::TextureFlags::RGBA8_NU_NEAR_LIN_EDGE});

	updateUniformsAfterViewResize();
	updateArrayBuffers();

	m_analysisTex.bind(TEX_UNIT_TILE_TRANSLU);
	m_shadowsTex.bind(TEX_UNIT_SHADOWS);
	m_analysisTex.bindImage(IMG_UNIT_TILE_TRANSLU, 0, RE::ImageAccess::READ_WRITE);
	m_shadowsTex.bindImage(IMG_UNIT_SHADOWS, 0, RE::ImageAccess::READ_WRITE);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(const glm::vec2& botLeftPx) {
	m_invBotLeftPx = botLeftPx;
	m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TILEPx)) - glm::ivec2(LIGHT_MAX_RANGETi);
	return ViewEnvelope{.botLeftTi = m_botLeftTi, .topRightTi = m_botLeftTi + glm::ivec2(m_viewSizeTi) + glm::ivec2(LIGHT_MAX_RANGETi) * 2};
}

void WorldDrawer::beginStep() {
	//Process the world texture to translucency texture
	m_analysisShd.setUniform(LOC_POSITION, m_botLeftTi);
	m_analysisShd.dispatchCompute(m_analysisGroupCount, true);
}

void WorldDrawer::addLight(const glm::vec2& posPx, RE::Color col) {
	//TODO
}

void WorldDrawer::endStep() {
	//Calculate shadow above each tile
	m_calcShadowsShd.dispatchCompute(m_calcShadowsGroupCount, true);
}

void WorldDrawer::drawTiles() {
	m_pouvArr.bind();
	m_drawTilesShd.use();
	m_drawTilesShd.setUniform(LOC_POSITION, m_invBotLeftPx);
	m_pouvArr.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewSizeTi.x * m_viewSizeTi.y);
	m_drawTilesShd.unuse();
	m_pouvArr.unbind();
}

void WorldDrawer::coverWithShadows() {
	if (m_drawShadows) {
		m_pouvArr.bind();
		m_drawShadowsShd.use();
		m_drawShadowsShd.setUniform(LOC_POSITION, glm::mod(m_invBotLeftPx, TILEPx));
		m_pouvArr.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewSizeTi.x * m_viewSizeTi.y);
		m_drawShadowsShd.unuse();
		m_pouvArr.unbind();
	}
}

void WorldDrawer::drawMinimap() {
	if (m_drawMinimap) {
		m_pouvArr.bind();
		m_minimapShd.use();
		m_pouvArr.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_MINIMAP_RECT, 4);
		m_minimapShd.unuse();
		m_pouvArr.unbind();
	}
}

void WorldDrawer::initVAOs() {
	//POUV vertex array
	GLuint vboBindingPoint = 0u;
	m_pouvArr.setBindingPoint(vboBindingPoint, m_pouvBuf, 0u, sizeof(VertexPOUV));

	m_pouvArr.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(VertexPOUV, position));
	m_pouvArr.setAttribute(RE::ATTR_UV, XY, FLOAT, offsetof(VertexPOUV, uv));

	m_pouvArr.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_pouvArr.connectAttributeToBindingPoint(RE::ATTR_UV, vboBindingPoint);
}

void WorldDrawer::updateArrayBuffers() {
	VertexPOUV vertices[8];

	//Normalized rectangle
	int i = VERTICES_POUV_NORM_RECT;
	vertices[i++] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
	vertices[i++] = {{1.0f, 0.0f}, {1.0f, 0.0f}};
	vertices[i++] = {{0.0f, 1.0f}, {0.0f, 1.0f}};
	vertices[i++] = {{1.0f, 1.0f}, {1.0f, 1.0f}};

	//Minimap rectangle
	float scale = std::min(m_viewSizePx.x / m_worldDimTi.x, m_viewSizePx.y / m_worldDimTi.y) * 0.5f;
	const glm::vec2 middle = m_viewSizePx * 0.5f;
	const glm::vec2 world = glm::vec2(m_worldDimTi) * scale;
	i = VERTICES_POUV_MINIMAP_RECT;
	vertices[i++] = {{middle.x - world.x, middle.y - world.y}, {0.0f, 0.0f}};
	vertices[i++] = {{middle.x + world.x, middle.y - world.y}, {1.0f, 0.0f}};
	vertices[i++] = {{middle.x - world.x, middle.y + world.y}, {0.0f, 1.0f}};
	vertices[i++] = {{middle.x + world.x, middle.y + world.y}, {1.0f, 1.0f}};

	m_pouvBuf.overwrite(0, sizeof(vertices), vertices);
}

void WorldDrawer::initShaders() {
	m_worldDrawUniformBuf.connectToInterfaceBlock(m_drawTilesShd, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToInterfaceBlock(m_drawTilesShd, 1u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToInterfaceBlock(m_drawShadowsShd, 0u);
	m_worldDrawUniformBuf.connectToInterfaceBlock(m_drawShadowsShd, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToInterfaceBlock(m_minimapShd, 0u);
	updateUniformsAfterViewResize();
}

void WorldDrawer::updateUniformsAfterViewResize() {
	WorldDrawUniforms wdu{
		.viewsizePxMat = glm::ortho(0.0f, m_viewSizePx.x, 0.0f, m_viewSizePx.y),
	};
	m_worldDrawUniformBuf.overwrite(0u, wdu);
	m_drawTilesShd.setUniform("viewWidthTi", static_cast<int>(m_viewSizeTi.x));
	m_drawShadowsShd.setUniform("viewWidthTi", static_cast<int>(m_viewSizeTi.x));
}