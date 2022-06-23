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

const GLint VERTICES_POUV_NORM_RECT = 0;
const GLint VERTICES_POUV_MINIMAP_RECT = 4;


WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx) {
	reloadViewSize(viewSizePx);
	m_surLighting.resize({m_viewDimsUn + glm::uvec2(LIGHT_MAX_RANGEUn) * 2u}, 3);

	//Bind textures to their reserved texture units
	m_blockAtlasTex->bind(TEX_UNIT_BLOCK_ATLAS);
	m_wallAtlasTex->bind(TEX_UNIT_WALL_ATLAS);
	m_blockLightAtlasTex->bind(TEX_UNIT_BLOCK_LIGHT_ATLAS);
	m_wallLightAtlasTex->bind(TEX_UNIT_WALL_LIGHT_ATLAS);
	m_surLighting.getTexture(0).bind(TEX_UNIT_LIGHTS_LIGHT);
	m_surLighting.getTexture(1).bind(TEX_UNIT_LIGHTS_TRANSLU);
	m_surLighting.getTexture(2).bind(TEX_UNIT_LIGHTS_COMPUTED);

	initShaders();
	initVAOs();
	updatePOUVBuffers();
}

WorldDrawer::~WorldDrawer() {

}

void WorldDrawer::setTarget(const glm::ivec2& worldDimTi) {
	m_worldDimTi = worldDimTi;

	updatePOUVBuffers();
}

void WorldDrawer::resizeView(const glm::uvec2& newViewSizePx) {
	reloadViewSize(newViewSizePx);
	updateUniformsAfterViewResize();
	updatePOUVBuffers();

	m_surLighting.resize({m_viewDimsUn + glm::uvec2(LIGHT_MAX_RANGEUn) * 2u}, 3);
	m_surLighting.getTexture(0).bind(TEX_UNIT_LIGHTS_LIGHT);
	m_surLighting.getTexture(1).bind(TEX_UNIT_LIGHTS_TRANSLU);
	m_surLighting.getTexture(2).bind(TEX_UNIT_LIGHTS_COMPUTED);
}

WorldDrawer::ViewEnvelope WorldDrawer::setPosition(const glm::vec2& botLeftPx) {
	m_invBotLeftPx = botLeftPx;
	m_botLeftTi = glm::ivec2(glm::floor(botLeftPx / TILEPx)) - glm::ivec2(LIGHT_MAX_RANGETi);
	return ViewEnvelope{.botLeftTi = m_botLeftTi, .topRightTi = m_botLeftTi + glm::ivec2(m_viewDimsTi) + glm::ivec2(LIGHT_MAX_RANGETi) * 2};
}

void WorldDrawer::beginStep() {
	//Process the world texture to translucency texture

}

void WorldDrawer::addLight(const glm::vec2& posPx, RE::Color col) {
	//TODO
}

void WorldDrawer::endStep() {
	//Calculate shadow of each light

}

void WorldDrawer::drawTiles() {
	m_pouvArray.bind();
	m_tilesShader.use();
	m_tilesShader.setUniform(LOC_POSITION, m_invBotLeftPx);
	m_pouvArray.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewDimsTi.x * m_viewDimsTi.y);
	m_tilesShader.unuse();
	m_pouvArray.unbind();
}

void WorldDrawer::coverWithShadows() {
	if (m_drawShadows) {
		m_pouvArray.bind();
		m_coverWithShadowsShader.use();
		m_coverWithShadowsShader.setUniform(LOC_POSITION, glm::mod(m_invBotLeftPx, TILEPx * LIGHT_DOWNSAMPLE));
		m_pouvArray.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewDimsUn.x * m_viewDimsUn.y);
		m_coverWithShadowsShader.unuse();
		m_pouvArray.unbind();
	}
}

void WorldDrawer::drawMinimap() {
	if (m_drawMinimap) {
		m_pouvArray.bind();
		m_minimapShader.use();
		m_pouvArray.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_MINIMAP_RECT, 4);
		m_minimapShader.unuse();
		m_pouvArray.unbind();
	}
}

void WorldDrawer::reloadViewSize(const glm::uvec2& viewSizePx) {
	m_viewDimsPx = (glm::vec2)viewSizePx;
	m_viewDimsTi = glm::uvec2(1u, 1u) + glm::uvec2(glm::ceil(m_viewDimsPx / TILEPx));
	m_viewDimsUn = glm::uvec2(1u, 1u) + glm::uvec2(glm::ceil(m_viewDimsPx / TILEPx / LIGHT_DOWNSAMPLE));
}

void WorldDrawer::initVAOs() {
	//POUV vertex array
	GLuint vboBindingPoint = 0u;
	m_pouvArray.setBindingPoint(vboBindingPoint, m_pouvBuffer, 0u, sizeof(VertexPOUV));

	m_pouvArray.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(VertexPOUV, position));
	m_pouvArray.setAttribute(RE::ATTR_UV, XY, FLOAT, offsetof(VertexPOUV, uv));

	m_pouvArray.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_pouvArray.connectAttributeToBindingPoint(RE::ATTR_UV, vboBindingPoint);
}

void WorldDrawer::updatePOUVBuffers() {
	VertexPOUV vertices[8];

	//Normalized rectangle
	int i = VERTICES_POUV_NORM_RECT;
	vertices[i++] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
	vertices[i++] = {{1.0f, 0.0f}, {1.0f, 0.0f}};
	vertices[i++] = {{0.0f, 1.0f}, {0.0f, 1.0f}};
	vertices[i++] = {{1.0f, 1.0f}, {1.0f, 1.0f}};

	//Minimap rectangle
	float scale = std::min(m_viewDimsPx.x / m_worldDimTi.x, m_viewDimsPx.y / m_worldDimTi.y) * 0.5f;
	const glm::vec2 middle = m_viewDimsPx * 0.5f;
	const glm::vec2 world = glm::vec2(m_worldDimTi) * scale;
	i = VERTICES_POUV_MINIMAP_RECT;
	vertices[i++] = {{middle.x - world.x, middle.y - world.y}, {0.0f, 0.0f}};
	vertices[i++] = {{middle.x + world.x, middle.y - world.y}, {1.0f, 0.0f}};
	vertices[i++] = {{middle.x - world.x, middle.y + world.y}, {0.0f, 1.0f}};
	vertices[i++] = {{middle.x + world.x, middle.y + world.y}, {1.0f, 1.0f}};

	m_pouvBuffer.overwrite(0, sizeof(vertices), vertices);
}

void WorldDrawer::initShaders() {
	m_worldDrawUniformBuffer.connectToInterfaceBlock(m_tilesShader, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToInterfaceBlock(m_tilesShader, 1u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToInterfaceBlock(m_coverWithShadowsShader, 0u);
	m_worldDrawUniformBuffer.connectToInterfaceBlock(m_coverWithShadowsShader, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToInterfaceBlock(m_minimapShader, 0u);
	updateUniformsAfterViewResize();
}

void WorldDrawer::updateUniformsAfterViewResize() {
	glm::vec2 worldToLightDims = glm::vec2(m_viewDimsUn) + glm::vec2(LIGHT_MAX_RANGEUn) * 2.0f;
	WorldDrawUniforms wdu{
		.viewsizePxMat = glm::ortho(0.0f, m_viewDimsPx.x, 0.0f, m_viewDimsPx.y),
	};
	m_worldDrawUniformBuffer.overwrite(0u, wdu);
	m_tilesShader.setUniform("viewWidthTi", static_cast<int>(m_viewDimsTi.x));
	m_coverWithShadowsShader.setUniform("viewWidthUn", static_cast<int>(m_viewDimsUn.x));
}