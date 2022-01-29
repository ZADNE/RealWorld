#include <RealWorld/world/World.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/main/Error.hpp>
#include <RealEngine/graphics/Viewport.hpp>

#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>
#include <RealWorld/shaders/WDS.hpp>


World::World() {
	initVAOs();
	initConstantUniforms();
	initUniformBuffers();
}

World::~World() {

}

void World::resizeWindow(const glm::vec2& newWindowDims) {
	rebuildDebugRectangle(newWindowDims);
}

void World::drawDebug() {
	if (m_shouldDrawDebug) {
		m_debugDraw.use();
		m_debugVAO.bind();

		m_debugVAO.renderArrays(TRIANGLE_STRIP, 0, 4);

		m_debugVAO.unbind();
		m_debugDraw.unuse();
	}
}

glm::uvec2 World::adoptWorldData(const WorldData& wd, const std::string& name, const glm::vec2& windowDims) {
	m_seed = wd.wi.seed;
	m_chunkDims = wd.wi.chunkDims;
	m_ws.resize(m_chunkDims * m_activeChunksRect, 1);
	m_worldName = name;

	TEX_UNIT_WORLD_TEXTURE.setActive();
	m_ws.bindTexture();
	TEX_UNIT_VOLATILE.setActive();

	m_chunkHandler.setTarget(m_seed, m_chunkDims, m_activeChunksRect, wd.path, &m_ws);

	rebuildDebugRectangle(windowDims);
	updateUniformsAfterWorldResize();

	return m_ws.getDims();
}

void World::gatherWorldData(WorldData& wd) const {
	wd.wi.chunkDims = m_chunkDims;
	wd.wi.seed = m_seed;
	wd.wi.worldName = m_worldName;
}

bool World::saveChunks() const {
	return m_chunkHandler.saveChunks();
}

int World::getNumberOfChunksLoaded() {
	return m_chunkHandler.getNumberOfChunksLoaded();
}

void World::forceActivationOfChunks(const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc) {
	m_chunkHandler.forceActivationOfChunks(botLeftBc, topRightBc);
}

uchar World::get(chunk::BLOCK_VALUES type, const glm::ivec2& posBc) {
	return m_chunkHandler.get(type, posBc);
}

uchar World::getMax(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc) {
	return m_chunkHandler.getMax(type, botLeftBc, dimBc);
}

uchar World::getMax(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc) {
	return m_chunkHandler.getMax(type, botLeftBc, topRightBc);
}

uchar World::getMin(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc) {
	return m_chunkHandler.getMin(type, botLeftBc, dimBc);
}

uchar World::getMin(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc) {
	return m_chunkHandler.getMin(type, botLeftBc, topRightBc);
}

void World::set(chunk::SET_TYPES type, const glm::ivec2& posBc, uchar index) {
	m_chunkHandler.set((chunk::BLOCK_VALUES)type, posBc, index);

	switch (type) {
	case chunk::SET_TYPES::BLOCK:
		glColorMask(true, true, false, false);
		break;
	case chunk::SET_TYPES::WALL:
		glColorMask(false, false, true, true);
		break;
	}

	m_ws.setTarget();
	m_setWithUpdateShader.use();
	m_setWithUpdateVAO.bind();

	m_setWithUpdateShader.setUniform(WGS::LOC_SET, (unsigned int)index);
	m_setWithUpdateShader.setUniform(WDS::LOC_POSITION, glm::vec2(static_cast<GLfloat>(posBc.x), static_cast<GLfloat>(posBc.y)));
	m_setWithUpdateShader.setUniform(shaders::LOC_TIME, ++m_time);

	m_setWithUpdateVAO.renderArrays(RE::Primitive::POINTS, 0, 9);

	m_setWithUpdateVAO.unbind();
	m_setWithUpdateShader.unuse();
	m_ws.resetTarget();

	glColorMask(true, true, true, true);
}

bool World::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc, uchar index) {
	return m_chunkHandler.exists(type, botLeftBc, dimBc, index);
}

bool World::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc, uchar index) {
	return m_chunkHandler.exists(type, botLeftBc, topRightBc, index);
}

void World::step() {
	m_chunkHandler.step();
}

void World::initVAOs() {
	//Set with var update
	GLuint vboBindingPoint = 0u;
	m_setWithUpdateVAO.setBindingPoint(vboBindingPoint, m_setWithUpdateVBO, 0u, sizeof(VertexSetWithUpdate));

	m_setWithUpdateVAO.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(VertexSetWithUpdate, position));
	m_setWithUpdateVAO.setAttribute(ATTR_SET_AROUND, X, UNSIGNED_INT, offsetof(VertexSetWithUpdate, setAround), false);

	m_setWithUpdateVAO.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_setWithUpdateVAO.connectAttributeToBindingPoint(ATTR_SET_AROUND, vboBindingPoint);

	//Debug
	m_debugVAO.setBindingPoint(vboBindingPoint, m_debugVBO, 0u, sizeof(VertexPOUV));

	m_debugVAO.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(VertexPOUV, position));
	m_debugVAO.setAttribute(RE::ATTR_UV, XY, FLOAT, offsetof(VertexPOUV, uv));

	m_debugVAO.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_debugVAO.connectAttributeToBindingPoint(RE::ATTR_UV, vboBindingPoint);
}

void World::rebuildDebugRectangle(const glm::vec2& newWindowDims) {
	float scale = 0.4f;
	const glm::vec2 middle = newWindowDims / 2.0f;
	const glm::vec2 world = m_ws.getDims();

	const VertexPOUV vertexData[4] = {
		{{middle.x - world.x * scale, middle.y + world.y * scale}, {0.0f, 0.0f}},
		{{middle.x + world.x * scale, middle.y + world.y * scale}, {1.0f, 0.0f}},
		{{middle.x - world.x * scale, middle.y - world.y * scale}, {0.0f, 1.0f}},
		{{middle.x + world.x * scale, middle.y - world.y * scale}, {1.0f, 1.0f}}
	};

	m_debugVBO.overwrite(0, sizeof(vertexData), vertexData);
}

void World::initConstantUniforms() {
	//SET WITH VAR UPDATE
	m_setWithUpdateShader.setUniform(shaders::LOC_AIR_ID, glm::uvec2((GLuint)BLOCK_ID::AIR, (GLuint)WALL_ID::AIR));
	m_setWithUpdateShader.setUniform(WGS::LOC_WORLD_TEXTURE, TEX_UNIT_WORLD_TEXTURE);

	//DEBUG DRAW shader
	m_debugDraw.setUniform(shaders::LOC_BASE_TEXTURE, TEX_UNIT_WORLD_TEXTURE);
}

void World::initUniformBuffers() {
	RE::Viewport::getWindowMatrixUniformBuffer().connectToShaderProgram(m_debugDraw, 0u);

	m_worldUniformBuffer.connectToShaderProgram(m_setWithUpdateShader, 0u);
}

void World::updateUniformsAfterWorldResize() {
	WorldUniforms wu{
		.worldMatrix = glm::ortho(0.0f, static_cast<float>(m_ws.getDims().x), 0.0f, static_cast<float>(m_ws.getDims().y)),
		.worldSize = glm::vec2(m_ws.getDims())
	};
	m_worldUniformBuffer.overwrite(0, sizeof(wu), &wu);
}