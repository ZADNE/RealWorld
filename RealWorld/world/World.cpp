#include <RealWorld/world/World.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/main/Error.hpp>
#include <RealEngine/graphics/UniformManager.hpp>

#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/world/WorldDataLoader.hpp>

struct WorldUniforms {
	glm::mat4 worldMatrix;
	glm::vec2 worldSize;
};


World::World(const glm::mat4& viewMatrix, const glm::uvec2& windowDims, RE::SpriteBatch& sb, Player& player) :
	m_windowDims(windowDims),
	m_player(player),
	m_fReg(sb, {1, 1}, * this, (glm::vec2)windowDims),
	m_worldDrawer(windowDims, viewMatrix, m_chunkHandler) {

	initVAOs();
	initConstantUniforms();
	initUniformBuffers();
}

World::~World() {

}

bool World::loadWorld(const std::string& worldName) {
	WorldData wd = WorldData();
	if (!WorldDataLoader::loadWorldData(wd, worldName)) return false;
	adoptWorldData(wd, worldName);
	return true;
}

bool World::saveWorld() const {
	WorldData wd;
	gatherWorldData(wd);
	if (!WorldDataLoader::saveWorldData(wd, m_worldName)) return false;
	return m_chunkHandler.saveChunks();
}

void World::resizeWindow(const glm::mat4& newViewMatrix, const glm::uvec2& newWindowDims) {
	m_windowDims = glm::vec2(newWindowDims);
	m_worldDrawer.resizeView(newWindowDims, newViewMatrix);
	m_fReg.resizeView((glm::vec2)newWindowDims);
	rebuildDebugRectangle();
}

void World::drawBeginStep() {
	m_worldDrawer.drawWorld();
	m_fReg.draw();
}

void World::drawEndStep() {
	if (m_shouldDrawDarkness) {
		m_worldDrawer.coverWithDarkness();
	}
	if (m_shouldDrawDebug) {
		drawDebug();
	}
}

void World::drawDebug() {
	m_debugDraw->use();
	m_debugVAO.bind();

	m_ws.bindTexture();
	m_debugVAO.renderArrays(RE::Primitive::TRIANGLES, 0, 6);

	m_debugVAO.unbind();
	m_debugDraw->unuse();
}

void World::adoptWorldData(const WorldData& wd, const std::string& name) {
	m_seed = wd.wi.seed;
	m_chunkDims = wd.wi.chunkDims;
	m_ws.resize(m_chunkDims * m_activeChunksRect, 1);
	m_worldName = name;
	m_worldDrawer.setTarget(m_ws.getDims(), m_ws.getTextureProxy());
	m_chunkHandler.setTarget(m_seed, m_chunkDims, m_activeChunksRect, wd.path, &m_ws);

	rebuildDebugRectangle();
	updateUniformsAfterWorldResize();

	m_player.adoptPlayerData(wd.pd);
	m_fReg.adoptFurnitureCollection(wd.fc);
}

void World::gatherWorldData(WorldData& wd) const {
	wd.wi.chunkDims = m_chunkDims;
	wd.wi.seed = m_seed;
	wd.wi.worldName = m_worldName;

	m_player.gatherPlayerData(wd.pd);
	m_fReg.gatherFurnitureCollection(wd.fc);
}

LightManipulator World::getLightManipulator() {
	return m_worldDrawer.getLightManipulator();
}

int World::getNumberOfChunksLoaded() {
	return m_chunkHandler.getNumberOfChunksLoaded();
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
	m_setWithUpdateShader->use();
	m_setWithUpdateVAO.bind();

	m_setWithUpdateShader->setUniform(WGS::LOC_SET, (unsigned int)index);
	m_setWithUpdateShader->setUniform(WDS::LOC_POSITION, glm::vec2(static_cast<GLfloat>(posBc.x), static_cast<GLfloat>(posBc.y)));
	m_setWithUpdateShader->setUniform(shaders::LOC_TIME, ++m_time);

	m_ws.bindTexture();

	m_setWithUpdateVAO.renderArrays(RE::Primitive::POINTS, 0, 9);

	m_setWithUpdateVAO.unbind();
	m_setWithUpdateShader->unuse();
	m_ws.resetTarget();

	glColorMask(true, true, true, true);

}

bool World::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc, uchar index) {
	return m_chunkHandler.exists(type, botLeftBc, dimBc, index);
}

bool World::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc, uchar index) {
	return m_chunkHandler.exists(type, botLeftBc, topRightBc, index);
}

FIndex World::build(const FStatic& furniture, bool actuallyBuild) {
	return m_fReg.build(furniture, actuallyBuild);
}

std::pair<size_t, glm::ivec2> World::destroy(const glm::ivec2& posSq) {
	return m_fReg.destroy(posSq);
}

void World::deallocateAll() {
	m_chunkHandler.flushChunks();
}

void World::beginStep(const glm::vec2& middlePosPx, const glm::vec2& botLeftPosPx) {
	m_chunkHandler.step();
	m_worldDrawer.beginStep(middlePosPx - m_windowDims / 2.0f);
	m_fReg.step(botLeftPosPx);
}

void World::endStep() {
	m_worldDrawer.endStep();
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

void World::rebuildDebugRectangle() {
	float scale = 0.4f;
	const glm::vec2 middle = m_windowDims / 2.0f;
	const glm::vec2 world = m_ws.getDims();

	const VertexPOUV vertexData[6] = {
		//Left-top triangle
		{{middle.x - world.x * scale, middle.y + world.y * scale}, {0.0f, 0.0f}},
		{{middle.x + world.x * scale, middle.y - world.y * scale}, {1.0f, 1.0f}},
		{{middle.x - world.x * scale, middle.y - world.y * scale}, {0.0f, 1.0f}},
		//Right-bottom triangle
		{{middle.x - world.x * scale, middle.y + world.y * scale}, {0.0f, 0.0f}},
		{{middle.x + world.x * scale, middle.y - world.y * scale}, {1.0f, 1.0f}},
		{{middle.x + world.x * scale, middle.y + world.y * scale}, {1.0f, 0.0f}},
	};

	m_debugVBO.overwrite(0, sizeof(vertexData), vertexData);
}

void World::initConstantUniforms() {
	//SET shader
	//nothing to do

	//SET WITH VAR UPDATE
	m_setWithUpdateShader->setUniform(shaders::LOC_AIR_ID, glm::uvec2((GLuint)BLOCK_ID::AIR, (GLuint)WALL_ID::AIR));
	m_setWithUpdateShader->setUniform(WGS::LOC_WORLD_TEXTURE, 0);

	//DEBUG DRAW shader
	m_debugDraw->setUniform(shaders::LOC_BASE_TEXTURE, 0);
}

void World::initUniformBuffers() {
	RE::UniformManager::std.addShader("GlobalMatrices", m_debugDraw.get());

	RE::UniformManager::std.addUniformBuffer("WorldUniforms", sizeof(WorldUniforms));
	RE::UniformManager::std.addShader("WorldUniforms", m_setWithUpdateShader.get());
}

void World::updateUniformsAfterWorldResize() {
	WorldUniforms wu;
	wu.worldSize = glm::vec2(m_ws.getDims());
	wu.worldMatrix = glm::ortho(0.0f, wu.worldSize.x, 0.0f, wu.worldSize.y);

	RE::UniformManager::std.setUniformBuffer("WorldUniforms", 0, sizeof(wu), &wu);
}