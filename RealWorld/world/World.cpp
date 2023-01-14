/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/World.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

 //Xorshift algorithm by George Marsaglia
uint32_t xorshift32(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

//Fisher–Yates shuffle algorithm
void permuteOrder(uint32_t& state, std::array<glm::ivec2, 4>& order) {
    for (size_t i = 0; i < order.size() - 1; i++) {
        size_t j = i + xorshift32(state) % (order.size() - i);
        std::swap(order[i].x, order[j].x);
        std::swap(order[i].y, order[j].y);
    }
}

struct TilePropertiesUIB {
    //x = properties
    //yz = indices of first and last transformation rule
    std::array<glm::uvec4, 256> blockTransformationProperties;
    std::array<glm::uvec4, 256> wallTransformationProperties;

    //x = The properties that neighbors MUST have to transform
    //y = The properties that neighbors MUST NOT have to transform
    //z = Properties of the transformation
    //w = The wall that it will be transformed into
    std::array<glm::uvec4, 16> blockTransformationRules;
    std::array<glm::uvec4, 16> wallTransformationRules;
};

static constexpr TilePropertiesUIB TILE_PROPERTIES = TilePropertiesUIB{
    .blockTransformationProperties = BLOCK_TRANSFORMATION_PROPERTIES,
    .wallTransformationProperties = WALL_TRANSFORMATION_PROPERTIES,
    .blockTransformationRules = BLOCK_TRANSFORMATION_RULES,
    .wallTransformationRules = WALL_TRANSFORMATION_RULES
};

World::World(ChunkGenerator& chunkGen):
    m_chunkManager(m_commandBuffer, chunkGen),
    m_tilePropertiesBuf(sizeof(TilePropertiesUIB), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, &TILE_PROPERTIES),
    m_rngState(static_cast<uint32_t>(time(nullptr))) {
    /*m_simulateFluidsShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDYNAMICS);
    m_transformTilesShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDYNAMICS);
    m_transformTilesShd.backInterfaceBlock(1u, UNIF_BUF_TILEPROPERTIES);
    m_modifyTilesShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDYNAMICS);
    m_transformTilesShd.backInterfaceBlock(0u, STRG_BUF_ACTIVECHUNKS);*/
}

const RE::Texture& World::adoptSave(const MetadataSave& save, const glm::ivec2& activeChunksArea) {
    m_seed = save.seed;
    m_worldName = save.worldName;

    //Resize the world texture
    glm::uvec2 texSize = iCHUNK_SIZE * activeChunksArea;
    using enum vk::ImageUsageFlagBits;
    m_worldTex = RE::Texture{RE::TextureCreateInfo{
        .extent = vk::Extent3D{texSize.x, texSize.y, 1u},
        .usage = eStorage | eTransferSrc | eTransferDst,
        .initialLayout = vk::ImageLayout::eGeneral
    }};

    m_chunkManager.setTarget(m_seed, save.path, *m_worldTex, activeChunksArea);

    return *m_worldTex;
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

void World::modify(LAYER layer, MODIFY_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile) {
    /*using enum RE::BufferMapUsageFlags;
    auto* buffer = m_worldDynamicsBuf.template map<WorldDynamicsUniforms>(0u, offsetof(WorldDynamicsUniforms, timeHash), WRITE | INVALIDATE_RANGE);
    buffer->globalPosTi = posTi;
    buffer->modifyTarget = static_cast<glm::uint>(layer);
    buffer->modifyShape = static_cast<glm::uint>(shape);
    buffer->modifyDiameter = diameter;
    buffer->modifySetValue = tile;
    m_worldDynamicsBuf.unmap();
    m_modifyTilesShd.dispatchCompute({1, 1, 1}, true);*/
}

int World::step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
    //Chunk manager
    int activatedChunks = m_chunkManager.forceActivationOfChunks(botLeftTi, topRightTi);
    m_chunkManager.step();

    //Tile transformations
    //m_transformTilesShd.dispatchCompute(offsetof(ActiveChunksSSBO, dynamicsGroupSize), true);
    //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);

    //Fluid dynamics
    //fluidDynamicsStep(botLeftTi, topRightTi);

    return activatedChunks;
}

void World::fluidDynamicsStep(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
    /*using enum RE::BufferMapUsageFlags;
    //Convert positions to chunks
    glm::ivec2 botLeftCh = tiToCh(botLeftTi);
    glm::ivec2 topRightCh = tiToCh(topRightTi);

    //Permute the orders
    m_simulateFluidsShd.use();
    if (m_permuteOrder) {
        auto* timeHash = m_worldDynamicsBuf.template map<glm::uint>(offsetof(WorldDynamicsUniforms, timeHash),
            sizeof(WorldDynamicsUniforms::timeHash) + sizeof(WorldDynamicsUniforms::updateOrder), WRITE | INVALIDATE_RANGE);
        *timeHash = m_rngState;
        glm::ivec2* updateOrder = reinterpret_cast<glm::ivec2*>(&timeHash[1]);
        //4 random orders, the threads randomly select from these
        for (unsigned int i = 0; i < 4; i++) {
            permuteOrder(m_rngState, m_dynamicsUpdateOrder);
            std::memcpy(&updateOrder[i * 4], &m_dynamicsUpdateOrder[0], 4 * sizeof(m_dynamicsUpdateOrder[0]));
        }
        m_worldDynamicsBuf.unmap();
        //Random order of dispatches
        permuteOrder(m_rngState, m_dynamicsUpdateOrder);
    }

    //4 rounds, each updates one quarter of the chunks
    glm::ivec2 dynBotLeftTi = botLeftCh * iCHUNK_SIZE + iCHUNK_SIZE / 2;
    for (unsigned int i = 0; i < 4u; i++) {
        //Update offset of the groups
        auto* offset = m_worldDynamicsBuf.template map<glm::ivec2>(0u, sizeof(glm::ivec2), WRITE | INVALIDATE_RANGE);
        *offset = dynBotLeftTi + glm::ivec2(m_dynamicsUpdateOrder[i]) * iCHUNK_SIZE / 2;
        m_worldDynamicsBuf.unmap();
        //Dispatch
        m_simulateFluidsShd.dispatchCompute({topRightCh - botLeftCh, 1u}, false);
        RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    }
    m_simulateFluidsShd.unuse();*/
}