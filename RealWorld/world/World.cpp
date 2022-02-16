﻿#include <RealWorld/world/World.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <RealEngine/main/Error.hpp>

#include <RealWorld/world/TDB.hpp>
#include <RealWorld/world/physics/Player.hpp>
#include <RealWorld/rendering/TextureUnits.hpp>
#include <RealWorld/rendering/ImageUnits.hpp>
#include <RealWorld/shaders/world_drawing.hpp>
#include <RealWorld/div.hpp>

//Xorshift algorithm by George Marsaglia
uint32_t xorshift32(uint32_t& state){
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

//Fisher–Yates shuffle algorithm
void permuteOrder(uint32_t& state, std::array<glm::ivec2, 4>& order) {
	for (int i = 0; i < order.size() - 1; i++){
		int j = i + xorshift32(state) % (order.size() - i);
		std::swap(order[i], order[j]);
	}
}

World::World():
	m_rngState(static_cast<uint32_t>(time(nullptr))) {
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
	m_worldName = name;

	m_worldSurface.getTexture(0).bind(TEX_UNIT_WORLD_TEXTURE);
	m_worldSurface.getTexture(0).bindImage(IMG_UNIT_WORLD, 0, RE::ImageAccess::READ_WRITE);

	m_chunkManager.setTarget(m_seed, m_activeChunksRect, wd.path, &m_worldSurface);

	updateUniformsAfterWorldResize();

	return m_worldSurface.getDims();
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

void World::set(SET_TARGET target, SET_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile) {
	m_chunkManager.set((TILE_VALUE)target, posTi, tile.x);

	m_modifyShader.setUniform(LOC_GLOBAL_OFFSET, posTi);
	m_modifyShader.setUniform(LOC_MODIFY_TARGET, static_cast<unsigned int>(target));
	m_modifyShader.setUniform(LOC_MODIFY_SHAPE, static_cast<unsigned int>(shape));
	m_modifyShader.setUniform(LOC_MODIFY_DIAMETER, diameter);
	m_modifyShader.setUniform(LOC_MODIFY_TILE, tile);
	unsigned int dispatchSize = glm::ceil(diameter / 8.0f);
	m_modifyShader.dispatchCompute({dispatchSize, dispatchSize, 1}, true);

	/*switch (type) {
	case SET_TARGET::BLOCK:
		glColorMask(true, true, false, false);
		break;
	case SET_TARGET::WALL:
		glColorMask(false, false, true, true);
		break;
	}

	m_worldSurface.setTarget();
	m_setWithUpdateShader.use();
	m_setWithUpdateVAO.bind();

	m_setWithUpdateShader.setUniform(LOC_SET, (unsigned int)index);
	m_setWithUpdateShader.setUniform(LOC_POSITION, glm::vec2(posTi));
	//m_setWithUpdateShader.setUniform(LOC_TIME, ++m_time);

	m_setWithUpdateVAO.renderArrays(RE::Primitive::POINTS, 0, 9);

	m_setWithUpdateVAO.unbind();
	m_setWithUpdateShader.unuse();
	m_worldSurface.resetTarget();

	glColorMask(true, true, true, true);*/
}

bool World::exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::uvec2& dimsTi, uchar index) {
	return m_chunkManager.exists(type, botLeftTi, dimsTi, index);
}

bool World::exists(TILE_VALUE type, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi, uchar index) {
	return m_chunkManager.exists(type, botLeftTi, topRightTi, index);
}

void World::step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	//Chunk manager
	m_chunkManager.forceActivationOfChunks(botLeftTi, topRightTi);
	m_chunkManager.step();
	/*static int i = 0;
	i++;
	if ((i % 50) == 0) {
		permuteOrder(m_rngState, m_dynamicsOrder);
	}*/
	//World dynamics
	static int i = 0;
	i++;
	//if ((i % 100) != 0) { return; }
	glm::ivec2 botLeftCh = floor_div(botLeftTi, CHUNK_SIZE).quot;
	glm::ivec2 topRightCh = floor_div(topRightTi, CHUNK_SIZE).quot;
	m_dynamicsShader.use();
	glm::ivec2 updateOrder[16];
	for (unsigned int i = 0; i < 4; i++){
		permuteOrder(m_rngState, m_dynamicsOrder);
		std::memcpy(&updateOrder[i * 4], &m_dynamicsOrder[0], 4 * sizeof(m_dynamicsOrder[0]));
	}
	m_dynamicsShader.setUniform(LOC_UPDATE_ORDER, 16, updateOrder);
	m_dynamicsShader.setUniform(LOC_TIME_HASH, m_rngState);
	glm::ivec2 dynBotLeftTi = botLeftCh * CHUNK_SIZE + CHUNK_SIZE / 2;
	permuteOrder(m_rngState, m_dynamicsOrder);
	for (int i = 0; i < m_dynamicsOrder.size(); i++) {
		m_dynamicsShader.setUniform(LOC_GLOBAL_OFFSET, dynBotLeftTi + m_dynamicsOrder[i] * CHUNK_SIZE / 2);
		m_dynamicsShader.dispatchCompute({(topRightCh - botLeftCh), 1u}, false);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	m_dynamicsShader.unuse();
}

void World::updateUniformsAfterWorldResize() {
	WorldUniforms wu{
		.worldMatrix = glm::ortho(0.0f, static_cast<float>(m_worldSurface.getDims().x), 0.0f, static_cast<float>(m_worldSurface.getDims().y)),
		.worldSize = glm::vec2(m_worldSurface.getDims())
	};
	m_worldUniformBuffer.overwrite(0, sizeof(wu), &wu);
}