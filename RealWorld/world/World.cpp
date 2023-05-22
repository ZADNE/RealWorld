/*!
 *  @author    Dubsky Tomas
 */
#include <RealWorld/world/World.hpp>

using S = vk::PipelineStageFlagBits2;
using A = vk::AccessFlagBits2;

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

static constexpr TilePropertiesUIB k_tileProperties = TilePropertiesUIB{
    .blockTransformationProperties = k_blockTransformationProperties,
    .wallTransformationProperties = k_wallTransformationProperties,
    .blockTransformationRules = k_blockTransformationRules,
    .wallTransformationRules = k_wallTransformationRules
};

World::World(ChunkGenerator& chunkGen):
    m_chunkManager(chunkGen),
    m_tilePropertiesBuf(sizeof(TilePropertiesUIB), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, &k_tileProperties),
    m_rngState(static_cast<uint32_t>(time(nullptr))) {
}

const RE::Texture& World::adoptSave(const MetadataSave& save, const glm::ivec2& activeChunksArea) {
    m_seed = save.seed;
    m_worldName = save.worldName;

    //Resize the world texture
    glm::uvec2 texSize = iChunkTi * activeChunksArea;
    using enum vk::ImageUsageFlagBits;
    m_worldTex = RE::Texture{RE::TextureCreateInfo{
        .format = vk::Format::eR8G8B8A8Uint,
        .extent = {texSize, 1u},
        .usage = eStorage | eTransferSrc | eTransferDst | eSampled
    }};

    m_chunkManager.setTarget(m_seed, save.path, *m_worldTex, activeChunksArea);

    return *m_worldTex;
}

void World::gatherSave(MetadataSave& save) const {
    save.seed = m_seed;
    save.worldName = m_worldName;
}

bool World::saveChunks() {
    return m_chunkManager.saveChunks();
}

size_t World::getNumberOfInactiveChunks() {
    return m_chunkManager.getNumberOfInactiveChunks();
}

void World::beginStep(const vk::CommandBuffer& commandBuffer) {
    //Transit world texture to general layout so that compute shaders can manipulate it
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader | S::eVertexShader,                                       //Src stage mask
        A::eShaderSampledRead | A::eShaderStorageRead | A::eShaderStorageWrite,     //Src access mask
        S::eComputeShader,                                                          //Dst stage mask
        A::eShaderStorageRead,                                                      //Dst access mask
        vk::ImageLayout::eReadOnlyOptimal,                                          //Old image layout
        vk::ImageLayout::eGeneral,                                                  //New image layout
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
        m_worldTex->image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u}
    };
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
}

int World::step(const vk::CommandBuffer& commandBuffer, const glm::ivec2& botLeftTi, const glm::ivec2& topRightTi) {
    //Chunk manager
    m_chunkManager.beginStep();
    m_chunkManager.planActivationOfChunks(commandBuffer, botLeftTi, topRightTi);
    int activatedChunks = m_chunkManager.endStep(commandBuffer);

    //Tile transformations
    //m_transformTilesShd.dispatchCompute(offsetof(ActiveChunksSSBO, dynamicsGroupSize), true);
    //RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);

    //Fluid dynamics
    //fluidDynamicsStep(botLeftTi, topRightTi);

    return activatedChunks;
}

void World::modify(const vk::CommandBuffer& commandBuffer, TileLayer layer, ModificationShape shape, float radius, const glm::ivec2& posTi, const glm::uvec2& tile) {
    //auto* buffer = m_worldDynamicsBuf.template map<WorldDynamicsUniforms>(0u, offsetof(WorldDynamicsUniforms, timeHash), WRITE | INVALIDATE_RANGE);
    /*buffer->globalPosTi = posTi;
    buffer->modifyTarget = static_cast<glm::uint>(layer);
    buffer->modifyShape = static_cast<glm::uint>(shape);
    buffer->modifyRadius = radius;
    buffer->modifySetValue = tile;
    m_worldDynamicsBuf.unmap();
    m_modifyTilesShd.dispatchCompute({1, 1, 1}, true);*/
}

void World::endStep(const vk::CommandBuffer& commandBuffer) {
    //Transit world texture back to readonly-optimal layout so that it can rendered
    auto imageBarrier = vk::ImageMemoryBarrier2{
        S::eComputeShader,                                                          //Src stage mask
        A::eShaderStorageRead | A::eShaderStorageWrite,                             //Src access mask
        S::eVertexShader,                                                           //Dst stage mask
        A::eShaderSampledRead,                                                      //Dst access mask
        vk::ImageLayout::eGeneral,                                                  //Old image layout
        vk::ImageLayout::eReadOnlyOptimal,                                          //New image layout
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,                           //Ownership transition
        m_worldTex->image(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u}
    };
    commandBuffer.pipelineBarrier2(vk::DependencyInfo{{}, {}, {}, imageBarrier});
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
    glm::ivec2 dynBotLeftTi = botLeftCh * iChunkTi + iChunkTi / 2;
    for (unsigned int i = 0; i < 4u; i++) {
        //Update offset of the groups
        auto* offset = m_worldDynamicsBuf.template map<glm::ivec2>(0u, sizeof(glm::ivec2), WRITE | INVALIDATE_RANGE);
        *offset = dynBotLeftTi + glm::ivec2(m_dynamicsUpdateOrder[i]) * iChunkTi / 2;
        m_worldDynamicsBuf.unmap();
        //Dispatch
        m_simulateFluidsShd.dispatchCompute({topRightCh - botLeftCh, 1u}, false);
        RE::Ordering::issueIncoherentAccessBarrier(SHADER_IMAGE_ACCESS);
    }
    m_simulateFluidsShd.unuse();*/
}