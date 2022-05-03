#include <RealWorld/world/World.hpp>

#include <RealWorld/rendering/TextureUnits.hpp>
#include <RealWorld/rendering/ImageUnits.hpp>

//Xorshift algorithm by George Marsaglia
uint32_t xorshift32(uint32_t& state) {
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

//Fisher–Yates shuffle algorithm
void permuteOrder(uint32_t& state, std::array<glm::ivec4, 4>& order) {
	for (size_t i = 0; i < order.size() - 1; i++) {
		size_t j = i + xorshift32(state) % (order.size() - i);
		std::swap(order[i].x, order[j].x);
		std::swap(order[i].y, order[j].y);
	}
}

World::World() :
	m_rngState(static_cast<uint32_t>(time(nullptr))) {
	m_worldDynamicsUBO.connectToShaderProgram(m_fluidDynamicsShader, 0u);
	m_worldDynamicsUBO.connectToShaderProgram(m_tileTransformationsShader, 0u);
	m_worldDynamicsUBO.connectToShaderProgram(m_modifyShader, 0u);
}

World::~World() {

}

glm::uvec2 World::adoptSave(const MetadataSave& save, const glm::vec2& windowDims) {
	m_seed = save.seed;
	m_worldName = save.worldName;

	m_worldSurface.getTexture(0).bind(TEX_UNIT_WORLD_TEXTURE);
	m_worldSurface.getTexture(0).bindImage(IMG_UNIT_WORLD, 0, RE::ImageAccess::READ_WRITE);
	m_worldSurface.getTexture(0).clear(RE::Color{0, 0, 0, 0});

	m_chunkManager.setTarget(m_seed, save.path, &m_worldSurface);

	return m_worldSurface.getDims();
}

void World::gatherSave(MetadataSave& save) const {
	save.seed = m_seed;
	save.worldName = m_worldName;
}

bool World::saveChunks() const {
	return m_chunkManager.saveChunks();
}

size_t World::getNumberOfInactiveChunks() {
	return m_chunkManager.getNumberOfInactiveChunks();
}

void World::set(SET_TARGET target, SET_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile) {
	auto* buffer = m_worldDynamicsUBO.map<WorldDynamicsUBO>(0u, offsetof(WorldDynamicsUBO, timeHash), WRITE | INVALIDATE_RANGE);
	buffer->globalPosTi = posTi;
	buffer->modifyTarget = static_cast<glm::uint>(target);
	buffer->modifyShape = static_cast<glm::uint>(shape);
	buffer->modifyDiameter = diameter;
	buffer->modifySetValue = tile;
	m_worldDynamicsUBO.unmap();
	m_modifyShader.dispatchCompute({1, 1, 1}, true);
}

void World::step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
	//Chunk manager
	m_chunkManager.forceActivationOfChunks(botLeftTi, topRightTi);
	m_chunkManager.step();

	//Tile transformations
	m_tileTransformationsShader.dispatchCompute(offsetof(ChunkManager::ActiveChunksSSBO, dynamicsGroupSize), true);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//Fluid dynamics
	glm::ivec2 botLeftCh = tiToCh(botLeftTi);
	glm::ivec2 topRightCh = tiToCh(topRightTi);
	m_fluidDynamicsShader.use();
	if (m_permuteOrder) {
		auto* timeHash = m_worldDynamicsUBO.map<glm::uint>(offsetof(WorldDynamicsUBO, timeHash),
			sizeof(WorldDynamicsUBO::timeHash) + sizeof(WorldDynamicsUBO::updateOrder), WRITE | INVALIDATE_RANGE);
		*timeHash = m_rngState;
		glm::ivec4* updateOrder = reinterpret_cast<glm::ivec4*>(&timeHash[1]);
		for (unsigned int i = 0; i < 4; i++) {
			permuteOrder(m_rngState, m_dynamicsUpdateOrder);
			std::memcpy(&updateOrder[i * 4], &m_dynamicsUpdateOrder[0], 4 * sizeof(m_dynamicsUpdateOrder[0]));
		}
		m_worldDynamicsUBO.unmap();
		permuteOrder(m_rngState, m_dynamicsUpdateOrder);
	}
	glm::ivec2 dynBotLeftTi = botLeftCh * iCHUNK_SIZE + iCHUNK_SIZE / 2;
	for (size_t i = 0; i < m_dynamicsUpdateOrder.size(); i++) {
		auto* offset = m_worldDynamicsUBO.map<glm::ivec2>(0u, sizeof(glm::ivec2), WRITE | INVALIDATE_RANGE);
		*offset = dynBotLeftTi + glm::ivec2(m_dynamicsUpdateOrder[i].x, m_dynamicsUpdateOrder[i].y) * iCHUNK_SIZE / 2;
		m_worldDynamicsUBO.unmap();
		m_fluidDynamicsShader.dispatchCompute({topRightCh - botLeftCh, 1u}, false);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	m_fluidDynamicsShader.unuse();
}