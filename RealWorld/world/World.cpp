/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/World.hpp>

#include <RealEngine/rendering/Ordering.hpp>

#include <RealWorld/reserved_units/textures.hpp>
#include <RealWorld/reserved_units/images.hpp>

using enum RE::IncoherentAccessBarrierFlags;

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

template<RE::Renderer R>
World<R>::World(ChunkGenerator<R>& chunkGen) :
    m_worldTex(RE::Raster{{1, 1}}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}),
    m_chunkManager(chunkGen),
    m_rngState(static_cast<uint32_t>(time(nullptr))) {
    m_simulateFluidsShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDYNAMICS);
    m_transformTilesShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDYNAMICS);
    m_modifyTilesShd.backInterfaceBlock(0u, UNIF_BUF_WORLDDYNAMICS);
    m_transformTilesShd.backInterfaceBlock(0u, STRG_BUF_ACTIVECHUNKS);
}

template<RE::Renderer R>
void World<R>::adoptSave(const MetadataSave& save, const glm::ivec2& activeChunksArea) {
    m_seed = save.seed;
    m_worldName = save.worldName;

    //Resize the world texture
    m_worldTex = RE::Texture<R>{{iCHUNK_SIZE * activeChunksArea}, {RE::TextureFlags::RGBA8_IU_NEAR_NEAR_EDGE}};
    m_worldTex.bind(TEX_UNIT_WORLD_TEXTURE);
    m_worldTex.bindImage(IMG_UNIT_WORLD, 0, RE::ImageAccess::READ_WRITE);
    m_worldTex.clear(RE::Color{0, 0, 0, 0});

    m_chunkManager.setTarget(m_seed, save.path, &m_worldTex);
}

template<RE::Renderer R>
void World<R>::gatherSave(MetadataSave& save) const {
    save.seed = m_seed;
    save.worldName = m_worldName;
}

template<RE::Renderer R>
bool World<R>::saveChunks() const {
    return m_chunkManager.saveChunks();
}

template<RE::Renderer R>
size_t World<R>::getNumberOfInactiveChunks() {
    return m_chunkManager.getNumberOfInactiveChunks();
}

template<RE::Renderer R>
void World<R>::modify(LAYER layer, MODIFY_SHAPE shape, float diameter, const glm::ivec2& posTi, const glm::uvec2& tile) {
    using enum RE::BufferMapUsageFlags;
    auto* buffer = m_worldDynamicsBuf.template map<WorldDynamicsUBO>(0u, offsetof(WorldDynamicsUBO, timeHash), WRITE | INVALIDATE_RANGE);
    buffer->globalPosTi = posTi;
    buffer->modifyTarget = static_cast<glm::uint>(layer);
    buffer->modifyShape = static_cast<glm::uint>(shape);
    buffer->modifyDiameter = diameter;
    buffer->modifySetValue = tile;
    m_worldDynamicsBuf.unmap();
    m_modifyTilesShd.dispatchCompute({1, 1, 1}, true);
}

template<RE::Renderer R>
int World<R>::step(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
    //Chunk manager
    int activatedChunks = m_chunkManager.forceActivationOfChunks(botLeftTi, topRightTi);
    m_chunkManager.step();

    //Tile transformations
    m_transformTilesShd.dispatchCompute(offsetof(typename ChunkManager<R>::ActiveChunksSSBO, dynamicsGroupSize), true);
    RE::Ordering<R>::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);

    //Fluid dynamics
    fluidDynamicsStep(botLeftTi, topRightTi);

    return activatedChunks;
}

template<RE::Renderer R>
void World<R>::fluidDynamicsStep(const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
    using enum RE::BufferMapUsageFlags;
    //Convert positions to chunks
    glm::ivec2 botLeftCh = tiToCh(botLeftTi);
    glm::ivec2 topRightCh = tiToCh(topRightTi);

    //Permute the orders
    m_simulateFluidsShd.use();
    if (m_permuteOrder) {
        auto* timeHash = m_worldDynamicsBuf.template map<glm::uint>(offsetof(WorldDynamicsUBO, timeHash),
            sizeof(WorldDynamicsUBO::timeHash) + sizeof(WorldDynamicsUBO::updateOrder), WRITE | INVALIDATE_RANGE);
        *timeHash = m_rngState;
        glm::ivec4* updateOrder = reinterpret_cast<glm::ivec4*>(&timeHash[1]);
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
        RE::Ordering<R>::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    }
    m_simulateFluidsShd.unuse();
}

template class World<RE::RendererVK13>;
template class World<RE::RendererGL46>;