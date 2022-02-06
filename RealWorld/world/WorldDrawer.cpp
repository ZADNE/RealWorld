#include <RealWorld/world/WorldDrawer.hpp>

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/Viewport.hpp>

#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/metadata.hpp>
#include <RealWorld/shaders/common.hpp>
#include <RealWorld/div.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>

const GLint VERTICES_POUV_WORLDTOLIGHT_RECT = 0;
const GLint VERTICES_POUV_COMPUTELIGHT_RECT = 4;
const GLint VERTICES_POUV_NORM_RECT = 8;
const GLint VERTICES_POUV_MINIMAP_RECT = 12;

const GLuint ATTR_DIR = 2u;
const GLuint ATTR_CONE = 3u;

const int LIGHT_DOWNSAMPLE = 4;
const int LIGHT_MAX_RANGEUn = 30;
const int LIGHT_MAX_RANGETi = LIGHT_MAX_RANGEUn * LIGHT_DOWNSAMPLE;


WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx) {
	reloadViewSize(viewSizePx);
	m_SurLighting.resize({m_viewsizeUn + glm::uvec2(LIGHT_MAX_RANGEUn) * 2u}, 3);

	//Bind textures to texture units
	m_blockAtlasTex->bind(TEX_UNIT_BLOCK_ATLAS);
	m_wallAtlasTex->bind(TEX_UNIT_WALL_ATLAS);
	m_SurLighting.bindTexture(TEX_UNIT_DIAPHRAGM, 0);
	m_SurLighting.bindTexture(TEX_UNIT_LIGHTING, 1);
	m_SurLighting.bindTexture(TEX_UNIT_LIGHT_FINISHED, 2);

	initShaders();
	initVAOs();
	updatePOUVBuffers();
}

WorldDrawer::~WorldDrawer() {

}

void WorldDrawer::setTarget(const glm::ivec2& worldDimTi) {
	m_worldDimTi = worldDimTi;

	glClearColor(m_backgroundColour.r, m_backgroundColour.g, m_backgroundColour.b, m_backgroundColour.a);
	m_addDynamicLightShader.setUniform("yInversion", m_worldDimTi.y * TILE_SIZE.y / LIGHT_DOWNSAMPLE);
	updatePOUVBuffers();
}

void WorldDrawer::resizeView(const glm::uvec2& newViewSizePx) {
	reloadViewSize(newViewSizePx);
	updateUniformsAfterViewResize();
	updatePOUVBuffers();

	m_SurLighting.resize({m_viewsizeUn + glm::uvec2(LIGHT_MAX_RANGEUn) * 2u}, 3);
	m_SurLighting.bindTexture(TEX_UNIT_DIAPHRAGM, 0);
	m_SurLighting.bindTexture(TEX_UNIT_LIGHTING, 1);
	m_SurLighting.bindTexture(TEX_UNIT_LIGHT_FINISHED, 2);
}

LightManipulator WorldDrawer::getLightManipulator() {
	return LightManipulator(this);
}

void WorldDrawer::beginStep(const glm::vec2& botLeftPx, World& world) {
	m_currentTime++;
	m_botLeftPx = botLeftPx;
	m_botLeftTi = glm::floor(botLeftPx / TILE_SIZE);
	//Force activation of chunks
	glm::ivec2 chunkHandlerBotleft = m_botLeftTi - glm::ivec2(LIGHT_MAX_RANGETi);
	world.forceActivationOfChunks(chunkHandlerBotleft, chunkHandlerBotleft
		+ static_cast<glm::ivec2>(m_viewsizeUn + glm::uvec2(LIGHT_MAX_RANGEUn) * 2u) * LIGHT_DOWNSAMPLE);

	m_botLeftTi += glm::ivec2(-LIGHT_MAX_RANGETi, LIGHT_MAX_RANGETi);
	glm::vec2 worldToLightPos = static_cast<glm::vec2>(m_botLeftTi) / LIGHT_DOWNSAMPLE;
	m_botLeftTi.y = m_worldDimTi.y - m_botLeftTi.y - m_viewsizeTi.y;

	worldToLightPos = glm::floor(worldToLightPos) * LIGHT_DOWNSAMPLE;
	worldToLightPos.y = m_worldDimTi.y - worldToLightPos.y - m_viewsizeTi.y;

	//Process world texture to diaphragm and light
	m_SurLighting.setTarget();
	glDisable(GL_BLEND);
	m_arrayPOUV.bind();
	m_worldToLightsShader.use();
	m_worldToLightsShader.setUniform(LOC_POSITION, worldToLightPos);
	float f = /*rmath::clamp(sin(m_currentTime / m_dayLength), (10.0f/256.0f), 1.0f)*/1.0f;
	m_worldToLightsShader.setUniform(LOC_WORLD_TO_LIGHT_DAYLIGHT, glm::vec4(0.0f, 0.0f, 0.0f, f * 0.1f));
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_WORLDTOLIGHT_RECT, 4);
	m_worldToLightsShader.unuse();
	m_arrayPOUV.unbind();
	glEnable(GL_BLEND);
	m_SurLighting.resetTarget();

	//Clear dynamic lights
	m_bufferDynamicLights.invalidate();
	m_dynamicLights.clear();
}

void WorldDrawer::endStep() {
	//Dynamic lights
	m_bufferDynamicLights.redefine(m_dynamicLights);
	m_SurLighting.setTarget();
	m_arrayLights.bind();
	m_addDynamicLightShader.use();
	glBlendFunc(GL_ONE, GL_ONE);
	glm::vec2 dynLightBotLeftTi = static_cast<glm::vec2>(m_botLeftTi);
	m_addDynamicLightShader.setUniform(LOC_POSITION, dynLightBotLeftTi);
	m_arrayLights.renderArrays(POINTS, 0, static_cast<GLsizei>(m_dynamicLights.size()), 4);
	glTextureBarrier();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_addDynamicLightShader.unuse();
	m_arrayLights.unbind();

	//Combine diaphragm and lighting texture to the finished texture
	glDisable(GL_BLEND);
	m_arrayPOUV.bind();
	m_computeLightingShader.use();
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_COMPUTELIGHT_RECT, 4);
	m_computeLightingShader.unuse();
	m_arrayPOUV.unbind();
	glEnable(GL_BLEND);
	m_SurLighting.resetTarget();
}

void WorldDrawer::drawTiles() {
	m_arrayPOUV.bind();
	m_tilesShader.use();
	m_tilesShader.setUniform(LOC_POSITION, m_botLeftPx);
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewsizeTi.x * m_viewsizeTi.y);
	m_tilesShader.unuse();
	m_arrayPOUV.unbind();
}

void WorldDrawer::coverWithDarkness() {
	if (m_drawDarkness) {
		m_arrayPOUV.bind();
		m_coverWithDarknessShader.use();
		m_coverWithDarknessShader.setUniform(LOC_POSITION, glm::mod(m_botLeftPx, TILE_SIZE * LIGHT_DOWNSAMPLE));
		m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewsizeUn.x * m_viewsizeUn.y);
		m_coverWithDarknessShader.unuse();
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

void WorldDrawer::addLight(const glm::vec2& posPx, RE::Colour col, float dir, float cone) {
	m_dynamicLights.emplace_back(posPx, col, dir, cone);
}

void WorldDrawer::reloadViewSize(const glm::uvec2& viewSizePx) {
	m_viewsizePx = (glm::vec2)viewSizePx;
	m_viewsizeTi = glm::uvec2(1u, 1u) + glm::uvec2(glm::ceil(m_viewsizePx / TILE_SIZE));
	m_viewsizeUn = glm::uvec2(1u, 1u) + glm::uvec2(glm::ceil(m_viewsizePx / TILE_SIZE / LIGHT_DOWNSAMPLE));
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
	m_arrayLights.setBindingPoint(vboBindingPoint, m_bufferDynamicLights, 0u, sizeof(DynamicLight));

	m_arrayLights.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(DynamicLight, posPx));
	m_arrayLights.setAttribute(RE::ATTR_COLOUR, RGBA, UNSIGNED_BYTE, offsetof(DynamicLight, col));
	m_arrayLights.setAttribute(ATTR_DIR, ONE, FLOAT, offsetof(DynamicLight, dir));
	m_arrayLights.setAttribute(ATTR_CONE, ONE, FLOAT, offsetof(DynamicLight, cone));

	m_arrayLights.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_arrayLights.connectAttributeToBindingPoint(RE::ATTR_COLOUR, vboBindingPoint);
	m_arrayLights.connectAttributeToBindingPoint(ATTR_DIR, vboBindingPoint);
	m_arrayLights.connectAttributeToBindingPoint(ATTR_CONE, vboBindingPoint);
}

void WorldDrawer::updatePOUVBuffers() {
	VertexPOUV vertices[16];

	//World to light rectangle
	size_t i = VERTICES_POUV_WORLDTOLIGHT_RECT;
	glm::vec2 worldToLightDims = glm::vec2(m_viewsizeUn) + glm::vec2(LIGHT_MAX_RANGEUn) * 2.0f;
	vertices[i++].setPosition(0.0f, 0.0f);
	vertices[i++].setPosition(worldToLightDims.x, 0.0f);
	vertices[i++].setPosition(0.0f, worldToLightDims.y);
	vertices[i++].setPosition(worldToLightDims.x, worldToLightDims.y);

	//Compute light rectangle
	i = VERTICES_POUV_COMPUTELIGHT_RECT;
	glm::vec2 computeLightTopRight = glm::vec2(m_viewsizeUn) + glm::vec2(LIGHT_MAX_RANGEUn + 1);
	vertices[i++].setPosition(LIGHT_MAX_RANGEUn, LIGHT_MAX_RANGEUn);
	vertices[i++].setPosition(computeLightTopRight.x, LIGHT_MAX_RANGEUn);
	vertices[i++].setPosition(LIGHT_MAX_RANGEUn, computeLightTopRight.y);
	vertices[i++].setPosition(computeLightTopRight.x, computeLightTopRight.y);

	//Normalized rectangle
	i = VERTICES_POUV_NORM_RECT;
	vertices[i++] = {0.0f, 0.0f, 0.0f, 1.0f};
	vertices[i++] = {1.0f, 0.0f, 1.0f, 1.0f};
	vertices[i++] = {0.0f, 1.0f, 0.0f, 0.0f};
	vertices[i++] = {1.0f, 1.0f, 1.0f, 0.0f};

	//Minimap rectangle
	float scale = std::min(m_viewsizePx.x / m_worldDimTi.x, m_viewsizePx.y / m_worldDimTi.y) * 0.5f;
	const glm::vec2 middle = m_viewsizePx / 2.0f;
	const glm::vec2 world = m_worldDimTi;
	i = VERTICES_POUV_MINIMAP_RECT;
	vertices[i++] = {{middle.x - world.x * scale, middle.y + world.y * scale}, {0.0f, 0.0f}};
	vertices[i++] = {{middle.x + world.x * scale, middle.y + world.y * scale}, {1.0f, 0.0f}};
	vertices[i++] = {{middle.x - world.x * scale, middle.y - world.y * scale}, {0.0f, 1.0f}};
	vertices[i++] = {{middle.x + world.x * scale, middle.y - world.y * scale}, {1.0f, 1.0f}};

	m_bufferPOUV.overwrite(0, sizeof(vertices), vertices);
}

void WorldDrawer::initShaders() {
	m_worldDrawUniformBuffer.connectToShaderProgram(m_tilesShader, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_tilesShader, 1u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_coverWithDarknessShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_coverWithDarknessShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_computeLightingShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_worldToLightsShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_addDynamicLightShader, 0u);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_minimapShader, 0u);
	updateUniformsAfterViewResize();

	m_addDynamicLightShader.setUniform("perPixelIncrementTi", 1.0f / TILE_SIZE);
	m_worldToLightsShader.setUniform(LOC_WORLD_TO_LIGHT_DIAPHRAGMS, glm::vec3(0.9f, 0.8f, 0.6f));
}

void WorldDrawer::updateUniformsAfterViewResize() {
	glm::vec2 worldToLightDims = glm::vec2(m_viewsizeUn) + glm::vec2(LIGHT_MAX_RANGEUn) * 2.0f;
	WorldDrawUniforms wdu{
		.viewsizePxMat = glm::ortho(0.0f, m_viewsizePx.x, 0.0f, m_viewsizePx.y),
		.viewsizeLightingUnMat = glm::ortho(0.0f, worldToLightDims.x, 0.0f, worldToLightDims.y)
	};
	m_worldDrawUniformBuffer.overwrite(wdu);
	m_tilesShader.setUniform("viewWidthTi", static_cast<int>(m_viewsizeTi.x));
	m_coverWithDarknessShader.setUniform("viewWidthUn", static_cast<int>(m_viewsizeUn.x));
}