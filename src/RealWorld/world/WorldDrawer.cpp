﻿#include <RealWorld/world/WorldDrawer.hpp>

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/Viewport.hpp>

#include <RealWorld/chunk/ChunkManager.hpp>
#include <RealWorld/constants/light.hpp>
#include <RealWorld/shaders/common.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>

const GLint VERTICES_POUV_WORLDTOLIGHT_RECT = 0;
const GLint VERTICES_POUV_COMPUTELIGHT_RECT = 4;
const GLint VERTICES_POUV_NORM_RECT = 8;
const GLint VERTICES_POUV_MINIMAP_RECT = 12;


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
	//Process the world texture to light and translucency
	m_surLighting.setTarget();
	glDisable(GL_BLEND);
	m_arrayPOUV.bind();
	m_tilesToLightsShader.use();
	glm::vec2 botLeftUn = static_cast<glm::vec2>(m_botLeftTi) / LIGHT_DOWNSAMPLE;
	botLeftUn = glm::floor(botLeftUn) * LIGHT_DOWNSAMPLE;
	m_tilesToLightsShader.setUniform(LOC_POSITION, botLeftUn);
	m_tilesToLightsShader.setUniform(LOC_TILES_TO_LIGHT_DAYLIGHT, glm::vec4(0.0f, 0.0f, 0.0f, 0.125f));
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_WORLDTOLIGHT_RECT, 4);
	m_tilesToLightsShader.unuse();
	m_arrayPOUV.unbind();
	glEnable(GL_BLEND);
	m_surLighting.resetTarget();

	//Clear lights
	m_bufferLights.invalidate();
	m_lights.clear();
}

void WorldDrawer::addLight(const glm::vec2& posPx, RE::Color col) {
	m_lights.emplace_back(posPx, col);
}

void WorldDrawer::endStep() {
	//Add lights
	m_bufferLights.redefine(m_lights);
	m_surLighting.setTarget();
	m_arrayLights.bind();
	m_addLightShader.use();
	glBlendFunc(GL_ONE, GL_ONE);
	glm::vec2 dynLightBotLeftUn = glm::floor(static_cast<glm::vec2>(m_botLeftTi) / LIGHT_DOWNSAMPLE);
	m_addLightShader.setUniform(LOC_POSITION, dynLightBotLeftUn);
	m_arrayLights.renderArrays(POINTS, 0, static_cast<GLsizei>(m_lights.size()), 4);
	glTextureBarrier();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_addLightShader.unuse();
	m_arrayLights.unbind();

	//Compute lighting
	glDisable(GL_BLEND);
	m_arrayPOUV.bind();
	m_computeLightingShader.use();
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_COMPUTELIGHT_RECT, 4);
	m_computeLightingShader.unuse();
	m_arrayPOUV.unbind();
	glEnable(GL_BLEND);
	m_surLighting.resetTarget();
}

void WorldDrawer::drawTiles() {
	m_arrayPOUV.bind();
	m_tilesShader.use();
	m_tilesShader.setUniform(LOC_POSITION, m_invBotLeftPx);
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewDimsTi.x * m_viewDimsTi.y);
	m_tilesShader.unuse();
	m_arrayPOUV.unbind();
}

void WorldDrawer::coverWithShadows() {
	if (m_drawShadows) {
		m_arrayPOUV.bind();
		m_coverWithShadowsShader.use();
		m_coverWithShadowsShader.setUniform(LOC_POSITION, glm::mod(m_invBotLeftPx, TILEPx * LIGHT_DOWNSAMPLE));
		m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewDimsUn.x * m_viewDimsUn.y);
		m_coverWithShadowsShader.unuse();
		m_arrayPOUV.unbind();
	}
}

void WorldDrawer::drawMinimap() {
	if (m_drawMinimap) {
		m_arrayPOUV.bind();
		m_minimapShader.use();
		m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_MINIMAP_RECT, 4);
		m_minimapShader.unuse();
		m_arrayPOUV.unbind();
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
	m_arrayPOUV.setBindingPoint(vboBindingPoint, m_bufferPOUV, 0u, sizeof(VertexPOUV));

	m_arrayPOUV.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(VertexPOUV, position));
	m_arrayPOUV.setAttribute(RE::ATTR_UV, XY, FLOAT, offsetof(VertexPOUV, uv));

	m_arrayPOUV.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_arrayPOUV.connectAttributeToBindingPoint(RE::ATTR_UV, vboBindingPoint);

	//Lights vertex array
	m_arrayLights.setBindingPoint(vboBindingPoint, m_bufferLights, 0u, sizeof(Light));

	m_arrayLights.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(Light, posPx));
	m_arrayLights.setAttribute(RE::ATTR_COLOR, RGBA, UNSIGNED_BYTE, offsetof(Light, col));

	m_arrayLights.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_arrayLights.connectAttributeToBindingPoint(RE::ATTR_COLOR, vboBindingPoint);
}

void WorldDrawer::updatePOUVBuffers() {
	VertexPOUV vertices[16];

	//World to light rectangle
	size_t i = VERTICES_POUV_WORLDTOLIGHT_RECT;
	glm::vec2 worldToLightDims = glm::vec2(m_viewDimsUn) + glm::vec2(LIGHT_MAX_RANGEUn) * 2.0f;
	vertices[i++].position = {0.0f, 0.0f};
	vertices[i++].position = {worldToLightDims.x, 0.0f};
	vertices[i++].position = {0.0f, worldToLightDims.y};
	vertices[i++].position = {worldToLightDims.x, worldToLightDims.y};

	//Compute light rectangle
	i = VERTICES_POUV_COMPUTELIGHT_RECT;
	glm::vec2 computeLightTopRight = glm::vec2(m_viewDimsUn) + glm::vec2(LIGHT_MAX_RANGEUn + 1);
	vertices[i++].position = {LIGHT_MAX_RANGEUn - 1, LIGHT_MAX_RANGEUn - 1};
	vertices[i++].position = {computeLightTopRight.x, LIGHT_MAX_RANGEUn - 1};
	vertices[i++].position = {LIGHT_MAX_RANGEUn - 1, computeLightTopRight.y};
	vertices[i++].position = {computeLightTopRight.x, computeLightTopRight.y};

	//Normalized rectangle
	i = VERTICES_POUV_NORM_RECT;
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

	m_bufferPOUV.overwrite(0, sizeof(vertices), vertices);
}

void WorldDrawer::initShaders() {
	m_worldDrawUniformBuffer.connectToShaderProgram(m_tilesShader, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_tilesShader, 1u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_coverWithShadowsShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_coverWithShadowsShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_computeLightingShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_tilesToLightsShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_addLightShader, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_minimapShader, 0u);
	updateUniformsAfterViewResize();

	m_tilesToLightsShader.setUniform(LOC_TILES_TO_LIGHT_DIAPHRAGMS, glm::vec3(0.9f, 0.8f, 0.6f));
}

void WorldDrawer::updateUniformsAfterViewResize() {
	glm::vec2 worldToLightDims = glm::vec2(m_viewDimsUn) + glm::vec2(LIGHT_MAX_RANGEUn) * 2.0f;
	WorldDrawUniforms wdu{
		.viewsizePxMat = glm::ortho(0.0f, m_viewDimsPx.x, 0.0f, m_viewDimsPx.y),
		.viewsizeLightingUnMat = glm::ortho(0.0f, worldToLightDims.x, 0.0f, worldToLightDims.y)
	};
	m_worldDrawUniformBuffer.overwrite(0u, wdu);
	m_tilesShader.setUniform("viewWidthTi", static_cast<int>(m_viewDimsTi.x));
	m_coverWithShadowsShader.setUniform("viewWidthUn", static_cast<int>(m_viewDimsUn.x));
}