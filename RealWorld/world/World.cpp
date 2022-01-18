#include <RealWorld/world/World.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/Vertex.hpp>
#include <RealEngine/Error.hpp>
#include <RealEngine/Vertex.hpp>
#include <RealEngine/Error.hpp>
#include <RealEngine/UniformManager.hpp>

#include <RealWorld/shaders/shaders.hpp>
#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/world/WorldDataLoader.hpp>

struct VertexSetWithVarUpdate {
	glm::vec2 position;
	unsigned int setAround;//bitfield informing about blocks added/removed around this block
};

struct WorldUniforms {
	glm::mat4 worldMatrix;
	glm::vec2 worldSize;
};


World::World() {

}

World::~World() {
	//Effect
	glDeleteVertexArrays(1, &m_effectVAO);
	glDeleteBuffers(1, &m_effectVBO);

	//Set with var update
	glDeleteVertexArrays(1, &m_setWithUpdateVAO);
	glDeleteBuffers(1, &m_setWithUpdateVBO);

	//Debug
	glDeleteVertexArrays(1, &m_debugVAO);
	glDeleteBuffers(1, &m_debugVBO);

	delete m_fReg;
	delete m_worldDrawer;
}

void World::init(const glm::mat4& viewMatrix, const glm::uvec2& windowDims, RE::SpriteBatch* sb, Player* player) {
	if (m_worldDrawer) return;//Anti-reinit check
	m_windowDim = glm::vec2(windowDims);
	m_player = player;


	m_worldDrawer = new WorldDrawer();
	m_worldDrawer->init(windowDims, viewMatrix, &m_chunkHandler);

	initOpenGLObjects();
	initConstantUniforms();
	initUniformBuffers();

	//FurnitureRegister
	m_fReg = new FurnitureRegister;
	m_fReg->init(sb, {1, 1}, this, (glm::vec2)windowDims);
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
	m_windowDim = glm::vec2(newWindowDims);
	m_worldDrawer->resizeView(newWindowDims, newViewMatrix);
	m_fReg->resizeView((glm::vec2)newWindowDims);
	rebuildDebugRectangle();
}

void World::drawBeginStep() {
	m_worldDrawer->drawWorld();
	m_fReg->draw();
}

void World::drawEndStep() {
	if (m_shouldDrawDarkness) {
		m_worldDrawer->coverWithDarkness();
	}
	if (m_shouldDrawDebug) {
		drawDebug();
	}
}

void World::drawDebug() {
	m_debugDraw->setShader();
	glBindVertexArray(m_debugVAO);
	m_ws.bindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	m_debugDraw->resetShader();
}

void World::adoptWorldData(const WorldData& wd, const std::string& name) {
	m_seed = wd.wi.seed;
	m_chunkDims = wd.wi.chunkDims;
	m_ws.resize(m_chunkDims * m_activeChunksRect, 1);
	m_worldName = name;
	m_worldDrawer->setTarget(m_ws.getDims(), m_ws.getTextureID());
	m_chunkHandler.setTarget(m_seed, m_chunkDims, m_activeChunksRect, wd.path, &m_ws);

	rebuildDebugRectangle();
	rebuildWorldRectangle();
	updateUniformsAfterWorldResize();

	m_player->adoptPlayerData(wd.pd);
	m_fReg->adoptFurnitureCollection(wd.fc);
}

void World::gatherWorldData(WorldData& wd) const {
	wd.wi.chunkDims = m_chunkDims;
	wd.wi.seed = m_seed;
	wd.wi.worldName = m_worldName;

	m_player->gatherPlayerData(wd.pd);
	m_fReg->gatherFurnitureCollection(wd.fc);
}

LightManipulator World::getLightManipulator() {
	return m_worldDrawer->getLightManipulator();
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

	m_ws.setTarget();
	switch (type) {
	case chunk::SET_TYPES::BLOCK:
		glColorMask(true, true, false, false);
		break;
	case chunk::SET_TYPES::WALL:
		glColorMask(false, false, true, true);
		break;
	}

	m_setWithVarUpdateShader->setShader();

	glBindVertexArray(m_setWithUpdateVAO);
	m_setWithVarUpdateShader->setUniform(WGS::LOC_SET, (unsigned int)index);
	m_setWithVarUpdateShader->setUniform(WDS::LOC_POSITION, glm::vec2(static_cast<GLfloat>(posBc.x), static_cast<GLfloat>(posBc.y)));
	m_setWithVarUpdateShader->setUniform(shaders::LOC_TIME, ++m_time);

	m_ws.bindTexture();

	glDrawArrays(GL_POINTS, 0, 9);

	glBindVertexArray(0);

	glColorMask(true, true, true, true);

	m_setWithVarUpdateShader->resetShader();
	m_ws.resetTarget();
}

bool World::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::uvec2& dimBc, uchar index) {
	return m_chunkHandler.exists(type, botLeftBc, dimBc, index);
}

bool World::exists(chunk::BLOCK_VALUES type, const glm::ivec2& botLeftBc, const glm::ivec2& topRightBc, uchar index) {
	return m_chunkHandler.exists(type, botLeftBc, topRightBc, index);
}

FIndex World::build(const FStatic& furniture, bool actuallyBuild) {
	return m_fReg->build(furniture, actuallyBuild);
}

std::pair<size_t, glm::ivec2> World::destroy(const glm::ivec2& posSq) {
	return m_fReg->destroy(posSq);
}

void World::deallocateAll() {
	m_chunkHandler.flushChunks();
}

void World::beginStep(const glm::vec2& middlePosPx, const glm::vec2& botLeftPosPx) {
	m_chunkHandler.step();
	m_worldDrawer->beginStep(middlePosPx - m_windowDim / 2.0f);
	m_fReg->step(botLeftPosPx);
}

void World::endStep() {
	m_worldDrawer->endStep();
}

void World::initOpenGLObjects() {
	//Effect
	glGenVertexArrays(1, &m_effectVAO);
	glGenBuffers(1, &m_effectVBO);
	glBindVertexArray(m_effectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_effectVBO);
	glEnableVertexAttribArray(RE::ATTLOC_PO);
	glVertexAttribPointer(RE::ATTLOC_PO, 2, GL_FLOAT, GL_FALSE, sizeof(RE::VertexPO), (void*)offsetof(RE::VertexPO, position));//Position
	glBindBuffer(GL_ARRAY_BUFFER, m_effectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RE::VertexPO) * 7, nullptr, GL_DYNAMIC_DRAW);

	//Set with var update
	glGenVertexArrays(1, &m_setWithUpdateVAO);
	glGenBuffers(1, &m_setWithUpdateVBO);

	glBindVertexArray(m_setWithUpdateVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_setWithUpdateVBO);

	glEnableVertexAttribArray(RE::ATTLOC_PO);//Position
	glVertexAttribPointer(RE::ATTLOC_PO, 2, GL_FLOAT, GL_FALSE, sizeof(VertexSetWithVarUpdate), (void*)offsetof(VertexSetWithVarUpdate, position));
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(VertexSetWithVarUpdate), (void*)offsetof(VertexSetWithVarUpdate, setAround));

	VertexSetWithVarUpdate vertices[9];
	unsigned int setAround[9] = {16, 32, 64, 8, 0, 128, 4, 2, 1};
	size_t i = 0;
	for (float x = -1.0f; x <= 1.0f; ++x) {
		for (float y = -1.0f; y <= 1.0f; ++y) {
			vertices[i].setAround = setAround[i];
			vertices[i++].position = {x, y};
		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

	//Debug
	glGenVertexArrays(1, &m_debugVAO);
	glGenBuffers(1, &m_debugVBO);

	glBindVertexArray(m_debugVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_debugVBO);

	glEnableVertexAttribArray(RE::ATTLOC_PO);//Position
	glVertexAttribPointer(RE::ATTLOC_PO, 2, GL_FLOAT, GL_FALSE, sizeof(RE::VertexPOUV), (void*)offsetof(RE::VertexPOUV, position));
	glEnableVertexAttribArray(RE::ATTLOC_UV);//UV
	glVertexAttribPointer(RE::ATTLOC_UV, 2, GL_FLOAT, GL_FALSE, sizeof(RE::VertexPOUV), (void*)offsetof(RE::VertexPOUV, uv));

	glBindVertexArray(0);
}

void World::rebuildDebugRectangle() {
	glBindBuffer(GL_ARRAY_BUFFER, m_debugVBO);
	float temp = 0.4f;

	RE::VertexPOUV vertexData[6];

	//Left-top triangle
	vertexData[0].setPosition(m_windowDim.x / 2.0f - (float)m_ws.getDims().x * temp, m_windowDim.y / 2.0f + (float)m_ws.getDims().y * temp);
	vertexData[0].setUV(0.0f, 0.0f);
	vertexData[1].setPosition(m_windowDim.x / 2.0f + (float)m_ws.getDims().x * temp, m_windowDim.y / 2.0f - (float)m_ws.getDims().y * temp);
	vertexData[1].setUV(1.0f, 1.0f);
	vertexData[2].setPosition(m_windowDim.x / 2.0f - (float)m_ws.getDims().x * temp, m_windowDim.y / 2.0f - (float)m_ws.getDims().y * temp);
	vertexData[2].setUV(0.0f, 1.0f);
	//Right-bottom triangle
	vertexData[3].setPosition(m_windowDim.x / 2.0f - (float)m_ws.getDims().x * temp, m_windowDim.y / 2.0f + (float)m_ws.getDims().y * temp);
	vertexData[3].setUV(0.0f, 0.0f);
	vertexData[4].setPosition(m_windowDim.x / 2.0f + (float)m_ws.getDims().x * temp, m_windowDim.y / 2.0f - (float)m_ws.getDims().y * temp);
	vertexData[4].setUV(1.0f, 1.0f);
	vertexData[5].setPosition(m_windowDim.x / 2.0f + (float)m_ws.getDims().x * temp, m_windowDim.y / 2.0f + (float)m_ws.getDims().y * temp);
	vertexData[5].setUV(1.0f, 0.0f);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
}

void World::rebuildWorldRectangle() {
	glBindBuffer(GL_ARRAY_BUFFER, m_effectVBO);
	glm::vec2 dims = glm::vec2(m_ws.getDims());
	RE::VertexPO vertices[6];
	//Left-top triangle
	vertices[0].setPosition(0.0f, 0.0f);
	vertices[1].setPosition(dims.x, dims.y);
	vertices[2].setPosition(0.0f, dims.y);
	//Right-bottom triangle
	vertices[3].setPosition(0.0f, 0.0f);
	vertices[4].setPosition(dims.x, dims.y);
	vertices[5].setPosition(dims.x, 0.0f);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &(vertices[0]));
}

void World::initConstantUniforms() {
	//SET shader
	//nothing to do

	//SET WITH VAR UPDATE
	m_setWithVarUpdateShader->setUniform(shaders::LOC_AIR_ID, glm::uvec2((GLuint)BLOCK_ID::AIR, (GLuint)WALL_ID::AIR));
	m_setWithVarUpdateShader->setUniform(WGS::LOC_WORLD_TEXTURE, 0);

	//DEBUG DRAW shader
	m_debugDraw->setUniform(shaders::LOC_BASE_TEXTURE, 0);
}

void World::initUniformBuffers() {
	RE::UniformManager::std.addShader("GlobalMatrices", m_debugDraw.get());

	RE::UniformManager::std.addUniformBuffer("WorldUniforms", sizeof(WorldUniforms));
	RE::UniformManager::std.addShader("WorldUniforms", m_setWithVarUpdateShader.get());
}

void World::updateUniformsAfterWorldResize() {
	WorldUniforms wu;
	wu.worldSize = glm::vec2(m_ws.getDims());
	wu.worldMatrix = glm::ortho(0.0f, wu.worldSize.x, 0.0f, wu.worldSize.y);

	RE::UniformManager::std.setUniformBuffer("WorldUniforms", 0, sizeof(wu), &wu);
}