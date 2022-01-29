#include <RealWorld/world/WorldDrawer.hpp>

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/resources/ResourceManager.hpp>
#include <RealEngine/graphics/Vertex.hpp>
#include <RealEngine/graphics/Surface.hpp>
#include <RealEngine/graphics/Viewport.hpp>

#include <RealWorld/world/ChunkManager.hpp>
#include <RealWorld/metadata.hpp>
#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/div.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>

const GLint VERTICES_POUV_WORLDTOLIGHT_RECT = 0;
const GLint VERTICES_POUV_DARKNESS_RECT = 4;
const GLint VERTICES_POUV_NORM_RECT = 8;

const GLuint ATTR_DIR = 2u;
const GLuint ATTR_CONE = 3u;

WorldDrawer::WorldDrawer(const glm::uvec2& viewSizePx) {
	reloadViewSize(viewSizePx);
	initShaders();
	initVAOs();
	updatePOUVBuffers();

	//Lighting
	m_SurLighting.resize({m_viewsizeLightingBc}, 3);

	TEX_UNIT_DIAPHRAGM.setActive();
	m_SurLighting.bindTexture(0);

	TEX_UNIT_LIGHTING.setActive();
	m_SurLighting.bindTexture(1);

	TEX_UNIT_LIGHT_FINISHED.setActive();
	m_SurLighting.bindTexture(2);

	TEX_UNIT_VOLATILE.setActive();

	//For dynamic lights
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 2.0f);


	TEX_UNIT_BLOCK_ATLAS.setActive();
	m_blockAtlasTex = RE::RM::getTexture("blockAtlas");
	m_blockAtlasTex->bind();

	TEX_UNIT_WALL_ATLAS.setActive();
	m_wallAtlasTex = RE::RM::getTexture("wallAtlas");
	m_wallAtlasTex->bind();

	TEX_UNIT_VOLATILE.setActive();
}

WorldDrawer::~WorldDrawer() {

}

void WorldDrawer::setTarget(const glm::ivec2& worldDimBc) {
	m_worldDimBc = worldDimBc;

	glClearColor(m_backgroundColour.r, m_backgroundColour.g, m_backgroundColour.b, m_backgroundColour.a);
	m_addDynamicLightShader.setUniform("yInversion", m_worldDimBc.y * vec2_BLOCK_SIZE.y);
}

void WorldDrawer::resizeView(const glm::uvec2& newViewSizePx) {
	reloadViewSize(newViewSizePx);
	updateUniformsAfterViewResize();
	updatePOUVBuffers();

	m_SurLighting.resize({m_viewsizeLightingBc}, 3);

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
	m_botLeftBc = floor_div((glm::ivec2)m_botLeftPx, ivec2_BLOCK_SIZE).quot;
	//Force activation of chunks
	glm::ivec2 chunkHandlerBotleft = m_botLeftBc - glm::ivec2((int)light::MAX_RANGE, (int)light::MAX_RANGE);
	world.forceActivationOfChunks(chunkHandlerBotleft, chunkHandlerBotleft + static_cast<glm::ivec2>(m_viewsizeLightingBc));

	m_botLeftBc -= glm::ivec2((int)light::MAX_RANGE, -(int)light::MAX_RANGE);
	m_botLeftBc.y = m_worldDimBc.y - m_botLeftBc.y - m_viewsizeBc.y;

	//Process world texture to diaphragm and light
	m_SurLighting.setTarget();
	glDisable(GL_BLEND);
	m_arrayPOUV.bind();
	m_worldToLightsShader.use();
	m_worldToLightsShader.setUniform(WDS::LOC_POSITION, static_cast<glm::vec2>(m_botLeftBc));
	float f = /*rmath::clamp(sin(m_currentTime / m_dayLength), (10.0f/256.0f), 1.0f)*/1.0f;
	m_worldToLightsShader.setUniform(WDS::LOC_WORLD_TO_LIGHT_DAYLIGHT, glm::vec4(0.0f, 0.0f, 0.0f, f * 0.1f));
	m_worldToLightsShader.setUniform(WDS::LOC_WORLD_TO_LIGHT_DIAPHRAGMS, glm::vec4(0.8f, 0.7f, 0.4f, 0.0f));
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
	glm::vec2 dynLightBotLeftBc = static_cast<glm::vec2>(m_botLeftBc) - glm::vec2(0.5, 0.5);//0.5 to shift to center of block
	m_addDynamicLightShader.setUniform(WDS::LOC_POSITION, dynLightBotLeftBc);
	m_arrayLights.renderArrays(POINTS, 0, static_cast<GLsizei>(m_dynamicLights.size()));
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
	m_tilesShader.setUniform(WDS::LOC_POSITION, m_botLeftPx);
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_NORM_RECT, 4, m_viewsizeBc.x * m_viewsizeBc.y);
	m_tilesShader.unuse();
	m_arrayPOUV.unbind();
}

void WorldDrawer::coverWithDarkness() {
	m_arrayPOUV.bind();
	m_coverWithDarknessShader.use();
	m_coverWithDarknessShader.setUniform(WDS::LOC_POSITION, glm::mod(m_botLeftPx, vec2_BLOCK_SIZE));
	m_arrayPOUV.renderArrays(TRIANGLE_STRIP, VERTICES_POUV_DARKNESS_RECT, 4);
	m_coverWithDarknessShader.unuse();
	m_arrayPOUV.unbind();
}

void WorldDrawer::addLight(const glm::vec2& posPx, RE::Colour col, float dir, float cone) {
	m_dynamicLights.emplace_back(posPx, col, dir, cone);
}

void WorldDrawer::reloadViewSize(const glm::uvec2& viewSizePx) {
	m_viewsizePx = (glm::vec2)viewSizePx;
	m_viewsizeBc = glm::uvec2(1u, 1u) + glm::uvec2(ceil(m_viewsizePx / vec2_BLOCK_SIZE));
	m_viewsizeLightingBc = m_viewsizeBc + glm::uvec2(light::MAX_RANGE * 2, light::MAX_RANGE * 2);
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
	VertexPOUV vertices[12];

	//Block size RECT
	size_t i = VERTICES_POUV_WORLDTOLIGHT_RECT;
	vertices[i++].setPosition(0.0f, 0.0f);
	vertices[i++].setPosition((float)m_viewsizeLightingBc.x, 0.0f);
	vertices[i++].setPosition(0.0f, (float)m_viewsizeLightingBc.y);
	vertices[i++].setPosition((float)m_viewsizeLightingBc.x, (float)m_viewsizeLightingBc.y);

	//DARKNESS RECT
	i = VERTICES_POUV_DARKNESS_RECT;
	glm::vec2 minn = glm::vec2((float)light::MAX_RANGE / m_viewsizeLightingBc.x, 1.0f - (float)light::MAX_RANGE / m_viewsizeLightingBc.y);
	glm::vec2 maxx = glm::vec2(1.0f, 1.0f) - minn;
	glm::vec2 darknessRect = glm::vec2(m_viewsizeBc * uvec2_BLOCK_SIZE);
	vertices[i++] = {0.0f, 0.0f, minn.x, minn.y};
	vertices[i++] = {darknessRect.x, 0.0f, maxx.x, minn.y};
	vertices[i++] = {0.0f, darknessRect.y, minn.x, maxx.y};
	vertices[i++] = {darknessRect.x, darknessRect.y, maxx.x, maxx.y};

	//Tiles mesh
	i = VERTICES_POUV_NORM_RECT;
	vertices[i++] = {0.0f, 0.0f, 0.0f, 1.0f};
	vertices[i++] = {1.0f, 0.0f, 1.0f, 1.0f};
	vertices[i++] = {0.0f, 1.0f, 0.0f, 0.0f};
	vertices[i++] = {1.0f, 1.0f, 1.0f, 0.0f};

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

	m_tilesShader.setUniform(WGS::LOC_WORLD_TEXTURE, TEX_UNIT_WORLD_TEXTURE);
	m_tilesShader.setUniform("blockTexture", TEX_UNIT_BLOCK_ATLAS);
	m_tilesShader.setUniform("wallTexture", TEX_UNIT_WALL_ATLAS);
	m_tilesShader.setUniform("blockSizePx", vec2_BLOCK_SIZE);
	m_tilesShader.setUniform("perTileUVIncrement", glm::vec2(1.0f / NUMBER_OF_TILES, 1.0f / NUMBER_OF_TILE_VARIATIONS));
	m_tilesShader.setUniform("viewWidthBc", static_cast<int>(m_viewsizeBc.x));

	m_coverWithDarknessShader.setUniform(shaders::LOC_BASE_TEXTURE, TEX_UNIT_LIGHT_FINISHED);

	m_computeLightingShader.setUniform(WDS::LOC_DIAPHRAGM, TEX_UNIT_DIAPHRAGM);
	m_computeLightingShader.setUniform(WDS::LOC_LIGHTING, TEX_UNIT_LIGHTING);

	m_worldToLightsShader.setUniform(WDS::LOC_WORLD_TEXTURE, TEX_UNIT_WORLD_TEXTURE);

	m_addDynamicLightShader.setUniform(WDS::LOC_DIAPHRAGM, TEX_UNIT_DIAPHRAGM);
	m_addDynamicLightShader.setUniform("invBlockSizePx", 1.0f / vec2_BLOCK_SIZE);
}

void WorldDrawer::updateUniformsAfterViewResize() {
	WorldDrawUniforms wdu{
		.viewsizePxMat = glm::ortho(0.0f, m_viewsizePx.x, 0.0f, m_viewsizePx.y),
		.viewsizeLightingBcMat = glm::ortho(0.0f, (float)m_viewsizeLightingBc.x, 0.0f, (float)m_viewsizeLightingBc.y)
	};
	m_worldDrawUniformBuffer.overwrite(wdu);
	m_tilesShader.setUniform("viewWidthBc", static_cast<int>(m_viewsizeBc.x));
}