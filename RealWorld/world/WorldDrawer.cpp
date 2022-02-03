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
const GLint VERTICES_POUV_DARKNESS_RECT = 4;
const GLint VERTICES_POUV_NORM_RECT = 8;
const GLint VERTICES_POUV_MINIMAP_RECT = 12;

const GLuint ATTR_DIR = 2u;
const GLuint ATTR_CONE = 3u;

WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx) {
	reloadViewSize(viewSizePx);
	m_SurLighting.resize({m_viewsizeLightingTi}, 3);

	//Bind textures to texture units
	TEX_UNIT_BLOCK_ATLAS.setActive();
	m_blockAtlasTex->bind();
	TEX_UNIT_WALL_ATLAS.setActive();
	m_wallAtlasTex->bind();
	TEX_UNIT_DIAPHRAGM.setActive();
	m_SurLighting.bindTexture(0);
	TEX_UNIT_LIGHTING.setActive();
	m_SurLighting.bindTexture(1);
	TEX_UNIT_LIGHT_FINISHED.setActive();
	m_SurLighting.bindTexture(2);
	TEX_UNIT_VOLATILE.setActive();

	initShaders();
	initVAOs();
	updatePOUVBuffers();
}

WorldDrawer::~WorldDrawer() {

}

void WorldDrawer::setTarget(const glm::ivec2& worldDimTi) {
	m_worldDimTi = worldDimTi;

	glClearColor(m_backgroundColour.r, m_backgroundColour.g, m_backgroundColour.b, m_backgroundColour.a);
	m_addDynamicLightShader.setUniform("yInversion", m_worldDimTi.y * TILE_SIZE.y);
	updatePOUVBuffers();
}

void WorldDrawer::resizeView(const glm::uvec2& newViewSizePx) {
	reloadViewSize(newViewSizePx);
	updateUniformsAfterViewResize();
	updatePOUVBuffers();

	m_SurLighting.resize({m_viewsizeLightingTi}, 3);

	TEX_UNIT_DIAPHRAGM.setActive();
	m_SurLighting.bindTexture(0);

	TEX_UNIT_LIGHTING.setActive();
	m_SurLighting.bindTexture(1);

	TEX_UNIT_LIGHT_FINISHED.setActive();
	m_SurLighting.bindTexture(2);

	TEX_UNIT_VOLATILE.setActive();
}

LightManipulator WorldDrawer::getLightManipulator() {
	return LightManipulator(this);
}

void WorldDrawer::beginStep(const glm::vec2& botLeftPx, World& world) {
	m_currentTime++;
	m_botLeftPx = botLeftPx;
	m_botLeftTi = glm::floor(botLeftPx / TILE_SIZE);
	//Force activation of chunks
	glm::ivec2 chunkHandlerBotleft = m_botLeftTi - glm::ivec2((int)light::MAX_RANGE, (int)light::MAX_RANGE);
	world.forceActivationOfChunks(chunkHandlerBotleft, chunkHandlerBotleft + static_cast<glm::ivec2>(m_viewsizeLightingTi));

	m_botLeftTi -= glm::ivec2((int)light::MAX_RANGE, -(int)light::MAX_RANGE);
	m_botLeftTi.y = m_worldDimTi.y - m_botLeftTi.y - m_viewsizeTi.y;

	//Process world texture to diaphragm and light
	m_SurLighting.setTarget();
	glDisable(GL_BLEND);
	m_arrayPOUV.bind();
	m_worldToLightsShader.use();
	m_worldToLightsShader.setUniform(LOC_POSITION, static_cast<glm::vec2>(m_botLeftTi));
	float f = /*rmath::clamp(sin(m_currentTime / m_dayLength), (10.0f/256.0f), 1.0f)*/1.0f;
	m_worldToLightsShader.setUniform(LOC_WORLD_TO_LIGHT_DAYLIGHT, glm::vec4(0.0f, 0.0f, 0.0f, f * 0.1f));
	m_worldToLightsShader.setUniform(LOC_WORLD_TO_LIGHT_DIAPHRAGMS, glm::vec4(0.85f, 0.75f, 0.5f, 0.0f));
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
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_WORLDTOLIGHT_RECT, 4);
	m_computeLightingShader.unuse();
	m_arrayPOUV.unbind();
	m_SurLighting.resetTarget();
	glEnable(GL_BLEND);
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
		m_coverWithDarknessShader.setUniform(LOC_POSITION, glm::mod(m_botLeftPx, TILE_SIZE));
		m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_DARKNESS_RECT, 4);
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
	m_viewsizeLightingTi = m_viewsizeTi + glm::uvec2(light::MAX_RANGE * 2, light::MAX_RANGE * 2);
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

	//Block size rectangle
	size_t i = VERTICES_POUV_WORLDTOLIGHT_RECT;
	vertices[i++].setPosition(0.0f, 0.0f);
	vertices[i++].setPosition((float)m_viewsizeLightingTi.x, 0.0f);
	vertices[i++].setPosition(0.0f, (float)m_viewsizeLightingTi.y);
	vertices[i++].setPosition((float)m_viewsizeLightingTi.x, (float)m_viewsizeLightingTi.y);

	//Darkness rectangle
	i = VERTICES_POUV_DARKNESS_RECT;
	glm::vec2 minn = glm::vec2((float)light::MAX_RANGE / m_viewsizeLightingTi.x, 1.0f - (float)light::MAX_RANGE / m_viewsizeLightingTi.y);
	glm::vec2 maxx = glm::vec2(1.0f, 1.0f) - minn;
	glm::vec2 darknessRect = uTILE_SIZE * m_viewsizeTi;
	vertices[i++] = {0.0f, 0.0f, minn.x, minn.y};
	vertices[i++] = {darknessRect.x, 0.0f, maxx.x, minn.y};
	vertices[i++] = {0.0f, darknessRect.y, minn.x, maxx.y};
	vertices[i++] = {darknessRect.x, darknessRect.y, maxx.x, maxx.y};

	//Normalized rectangle
	i = VERTICES_POUV_NORM_RECT;
	vertices[i++] = {0.0f, 0.0f, 0.0f, 1.0f};
	vertices[i++] = {1.0f, 0.0f, 1.0f, 1.0f};
	vertices[i++] = {0.0f, 1.0f, 0.0f, 0.0f};
	vertices[i++] = {1.0f, 1.0f, 1.0f, 0.0f};

	//Minimap rectangle
	float scale = 0.6f;
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
	m_worldDrawUniformBuffer.connectToShaderProgram(m_coverWithDarknessShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_computeLightingShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_worldToLightsShader, 0u);
	m_worldDrawUniformBuffer.connectToShaderProgram(m_addDynamicLightShader, 0u);
	updateUniformsAfterViewResize();

	m_tilesShader.setUniform(LOC_WORLD_TEXTURE, TEX_UNIT_WORLD_TEXTURE);
	m_tilesShader.setUniform("blockTexture", TEX_UNIT_BLOCK_ATLAS);
	m_tilesShader.setUniform("wallTexture", TEX_UNIT_WALL_ATLAS);
	m_tilesShader.setUniform("tileSizePx", TILE_SIZE);
	glm::vec4 atlasDims{m_blockAtlasTex->getTrueDims(), m_wallAtlasTex->getTrueDims()};
	m_tilesShader.setUniform("perTileUVIncrement", 1.0f / (atlasDims / glm::vec4(TILE_SIZE, TILE_SIZE)));
	m_tilesShader.setUniform("viewWidthTi", static_cast<int>(m_viewsizeTi.x));

	m_coverWithDarknessShader.setUniform(LOC_BASE_TEXTURE, TEX_UNIT_LIGHT_FINISHED);

	m_computeLightingShader.setUniform(LOC_DIAPHRAGM, TEX_UNIT_DIAPHRAGM);
	m_computeLightingShader.setUniform(LOC_LIGHTING, TEX_UNIT_LIGHTING);

	m_worldToLightsShader.setUniform(LOC_WORLD_TEXTURE, TEX_UNIT_WORLD_TEXTURE);

	m_addDynamicLightShader.setUniform("perPixelIncrementTi", 1.0f / TILE_SIZE);

	m_minimapShader.setUniform(LOC_BASE_TEXTURE, TEX_UNIT_WORLD_TEXTURE);
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_minimapShader, 0u);
}

void WorldDrawer::updateUniformsAfterViewResize() {
	WorldDrawUniforms wdu{
		.viewsizePxMat = glm::ortho(0.0f, m_viewsizePx.x, 0.0f, m_viewsizePx.y),
		.viewsizeLightingTiMat = glm::ortho(0.0f, (float)m_viewsizeLightingTi.x, 0.0f, (float)m_viewsizeLightingTi.y)
	};
	m_worldDrawUniformBuffer.overwrite(wdu);
	m_tilesShader.setUniform("viewWidthTi", static_cast<int>(m_viewsizeTi.x));
}