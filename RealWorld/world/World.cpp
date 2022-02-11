#include <RealWorld/world/World.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/main/Error.hpp>

#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>
#include <RealWorld/rendering/ImageUnits.hpp>
#include <RealWorld/shaders/world_drawing.hpp>

World::World() {
	GLuint vboBindingPoint = 0u;
	m_setWithUpdateVAO.setBindingPoint(vboBindingPoint, m_setWithUpdateVBO, 0u, sizeof(VertexSetWithUpdate));
	m_setWithUpdateVAO.setAttribute(RE::ATTR_POSITION, XY, FLOAT, offsetof(VertexSetWithUpdate, position));
	m_setWithUpdateVAO.setAttribute(ATTR_SET_AROUND, X, UNSIGNED_INT, offsetof(VertexSetWithUpdate, setAround), false);
	m_setWithUpdateVAO.connectAttributeToBindingPoint(RE::ATTR_POSITION, vboBindingPoint);
	m_setWithUpdateVAO.connectAttributeToBindingPoint(ATTR_SET_AROUND, vboBindingPoint);

	m_worldUniformBuffer.connectToShaderProgram(m_setWithUpdateShader, 0u);
}

World::~World() {

}

glm::uvec2 World::adoptWorldData(const WorldData& wd, const std::string& name, const glm::vec2& windowDims) {
	m_seed = wd.wi.seed;
	m_ws.resize(glm::uvec2(CHUNK_SIZE) * m_activeChunksRect, 1);
	m_worldName = name;

	m_ws.getTexture(0).bind(TEX_UNIT_WORLD_TEXTURE);
	m_ws.getTexture(0).bindImage(IMG_UNIT_WORLD, 0, RE::ImageAccess::READ_WRITE);

	m_chunkManager.setTarget(m_seed, m_activeChunksRect, wd.path, &m_ws);

	updateUniformsAfterWorldResize();

	return m_ws.getDims();
}

void World::gatherWorldData(WorldData& wd) const {
	wd.wi.seed = m_seed;
	wd.wi.worldName = m_worldName;
}

bool World::saveChunks() const {
	return m_chunkManager.saveChunks();
}

size_t World::getNumberOfChunksLoaded() {
	return m_chunkManager.getNumberOfChunksLoaded();
}

void World::forceActivationOfChunks(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	m_chunkManager.forceActivationOfChunks(botLeftTi, topRightTi);
}

uchar World::get(TILE_VALUE type, const glm::ivec2& posTi) {
	return m_chunkManager.get(type, posTi);
}

uchar World::getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi) {
	return m_chunkManager.getMax(type, botLeftTi, dimsTi);
}

uchar World::getMax(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	return m_chunkManager.getMax(type, botLeftTi, topRightTi);
}

uchar World::getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi) {
	return m_chunkManager.getMin(type, botLeftTi, dimsTi);
}

uchar World::getMin(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	return m_chunkManager.getMin(type, botLeftTi, topRightTi);
}

void World::set(SET_TYPES type, const glm::ivec2& posTi, uchar index) {
	m_chunkManager.set((TILE_VALUE)type, posTi, index);

	switch (type) {
	case SET_TYPES::BLOCK:
		glColorMask(true, true, false, false);
		break;
	case SET_TYPES::WALL:
		glColorMask(false, false, true, true);
		break;
	}

	m_ws.setTarget();
	m_setWithUpdateShader.use();
	m_setWithUpdateVAO.bind();

	m_setWithUpdateShader.setUniform(LOC_SET, (unsigned int)index);
	m_setWithUpdateShader.setUniform(LOC_POSITION, glm::vec2(posTi));
	m_setWithUpdateShader.setUniform(LOC_TIME, ++m_time);

	m_setWithUpdateVAO.renderArrays(RE::Primitive::POINTS, 0, 9);

	m_setWithUpdateVAO.unbind();
	m_setWithUpdateShader.unuse();
	m_ws.resetTarget();

	glColorMask(true, true, true, true);
}

bool World::exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi, uchar index) {
	return m_chunkManager.exists(type, botLeftTi, dimsTi, index);
}

bool World::exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi, uchar index) {
	return m_chunkManager.exists(type, botLeftTi, topRightTi, index);
}

void World::step() {
	m_chunkManager.step();
	//m_dynamicsShader.dispatchCompute({(m_activeChunksRect * CHUNK_SIZE) / DYN_CHUNK_SIZETi, 1u});
}

void World::updateUniformsAfterWorldResize() {
	WorldUniforms wu{
		.worldMatrix = glm::ortho(0.0f, static_cast<float>(m_ws.getDims().x), 0.0f, static_cast<float>(m_ws.getDims().y)),
		.worldSize = glm::vec2(m_ws.getDims())
	};
	m_worldUniformBuffer.overwrite(0, sizeof(wu), &wu);
}